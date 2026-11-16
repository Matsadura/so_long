#!/usr/bin/env bash
# Headless regression tests for the map validator and core game loop.
# Usage: tests/run_tests.sh            (VALGRIND=1 to run every case under valgrind)
set -u
cd "$(dirname "$0")/.."

BIN=./so_long
export SDL_VIDEODRIVER="${SDL_VIDEODRIVER:-dummy}"
export SDL_AUDIODRIVER=dummy
pass=0
fail=0

run() {
	if [ "${VALGRIND:-0}" = 1 ]; then
		valgrind -q --leak-check=full --errors-for-leak-kinds=definite,indirect \
			--track-fds=yes --error-exitcode=99 --num-callers=40 \
			--suppressions=tests/valgrind.supp --gen-suppressions=all "$@"
	else
		"$@"
	fi
}

ok()   { pass=$((pass + 1)); printf '  \033[32mPASS\033[0m %s\n' "$1"; }
ko()   {
	fail=$((fail + 1))
	printf '  \033[31mFAIL\033[0m %s\n' "$1"
	[ -n "${2:-}" ] && printf '       %s\n' "$2"
	# Under valgrind, show the whole report (and generated suppressions).
	if [ "${VALGRIND:-0}" = 1 ] && grep -q '^==' <<<"${out:-}"; then
		printf '%s\n' "$out" | sed 's/^/       | /'
	fi
	return 0
}

echo "== valid maps (expect OK)"
for m in tests/maps/valid/*.ber assets/maps/*.ber; do
	out=$(run "$BIN" --check "$m" 2>&1); rc=$?
	[ $rc -eq 0 ] && ok "$m" || ko "$m" "$out"
done

echo "== invalid maps (expect clean error, exit 1)"
for m in tests/maps/invalid/*.ber tests/maps/bad_names/* tests/maps/bad_names/.ber \
	tests/maps/does_not_exist.ber; do
	out=$(run "$BIN" --check "$m" 2>&1); rc=$?
	if [ $rc -eq 1 ] && grep -q "Error:" <<<"$out"; then ok "$m -> ${out#*Error: }"
	else ko "$m" "rc=$rc $out"; fi
done

echo "== gameplay (scripted input, dummy video driver, fixed 25 Hz steps)"
# Keys: e enter, wasd hop, r retry, p pause, . wait one frame.
# Every gameplay run starts on the title screen, hence the leading "e".
play() { # name expected_status_substring args...
	local name=$1 expect=$2; shift 2
	out=$(run "$BIN" --dt 0.04 "$@" 2>&1); rc=$?
	if [ $rc -eq 0 ] && grep -q -- "$expect" <<<"$out"; then ok "$name"
	else ko "$name" "rc=$rc $(tail -n 3 <<<"$out")"; fi
}
wait_frames() { printf '%*s' "$1" '' | tr ' ' '.'; }
C1=assets/maps/classic/classic1.ber
C2=assets/maps/classic/classic2.ber
T=tests/maps/valid
W30=$(wait_frames 30)

# Title screen
play "starts on the title screen"     "state=TITLE" --frames 5
play "enter starts a run"             "state=PLAYING lives=3" --keys "e" --frames 5

# Classic so_long rules still work on lane-less maps
play "walls block, moves not counted" "moves=0" --keys "ea..a..a.." --frames 12 $T/min.ber
play "classic: coins then exit"       "state=VICTORY lives=3 score=.* moves=9 coins=2/2" \
	--keys "es..w..w..d..d..d..d..s..s.." --frames 32 $C1
play "classic: exit closed w/o coins" "state=PLAYING" --keys "es..d..d..d..d..d.." --frames 24 $C1
play "campaign advances on ENTER"     "level=2 state=PLAYING" \
	--keys "es..w..w..d..d..d..d..s..s..e" --frames 34 $C1 $C2
# Player walks next to the patrol; it turns at the exit and steps back
# onto the player at 1.35s.
play "enemy contact costs a life"     "state=DYING lives=3" --keys "ed..d..d..d.." --frames 40 $C2
play "life lost after death anim"     "state=PLAYING lives=2" --keys "ed..d..d..d.." --frames 75 $C2
play "coin combo multiplier"          "score=300 moves=3 coins=3/3" --keys "ed..d..d.." --frames 12 $T/combo.ber

# Frogger lanes
play "cross road + ride log to goal"  "state=VICTORY lives=3 score=1520 moves=3 coins=0/0 goals=1/1" \
	--keys "ew..w..w..." --frames 20 $T/frog_cross.ber
play "car hit = SPLAT"                "state=DYING" --keys "ew" --frames 8 $T/frog_car.ber
play "log carries player (2.0s)"      "state=PLAYING" --keys "ew..w" --frames 50 $T/frog_cross.ber
play "swept into wall = SPLASH"       "state=DYING" --keys "ew..w" --frames 70 $T/frog_cross.ber
play "retry restores stage lives"     "state=PLAYING lives=3 score=0 moves=0" \
	--keys "ew.r" --frames 8 $T/frog_car.ber
# 3 x (30s timer + 1.1s death animation) = 93.3s of game time at 10 Hz.
play "three deaths = game over"       "state=GAME_OVER lives=0" \
	--keys "e" --dt 0.1 --frames 950 $T/frog_car.ber
play "game over -> instant new run"   "state=PLAYING lives=3 score=0" \
	--keys "e$(wait_frames 945)e" --dt 0.1 --frames 950 $T/frog_car.ber
play "pause freezes the game"         "state=PAUSED lives=3" --keys "ep" --frames 200 $T/frog_car.ber
play "timer runs out = TIME UP"       "state=DYING lives=3" --keys "e" --frames 770 $T/frog_car.ber

# Frame-rate independence: the patrol enemy reaches the idle player after
# 7 steps * 0.45s = 3.15s of game time at 60 Hz and 240 Hz alike.
play "enemy idle at 2.0s @60Hz"       "state=PLAYING" --keys "e" --dt 0.016666 --frames 120 $C2
play "enemy idle at 2.0s @240Hz"      "state=PLAYING" --keys "e" --dt 0.0041666 --frames 480 $C2
play "enemy hits at 3.5s @60Hz"       "state=DYING" --keys "e" --dt 0.016666 --frames 210 $C2
play "enemy hits at 3.5s @240Hz"      "state=DYING" --keys "e" --dt 0.0041666 --frames 840 $C2
# Audio (SDL dummy audio driver): every clip loads, mute toggles, and the
# sound events of a full crossing + game over play without errors.
out=$(run "$BIN" --dt 0.04 --keys "em" --frames 5 2>&1); rc=$?
if [ $rc -eq 0 ] && ! grep -q "Warning: cannot load" <<<"$out"; then ok "all sounds load"
else ko "all sounds load" "rc=$rc $(grep -m3 Warning <<<"$out")"; fi
play "mute toggles without side effects" "state=PLAYING lives=3" --keys "emm" --frames 5
play "every sound event plays"        "state=VICTORY" --keys "ew..w..w...p.p" --frames 25 $T/frog_cross.ber
play "full campaign loads"            "level=1 state=PLAYING" --keys "e" --frames 3
play "ESC/quit exits cleanly"         "level=1" --keys "eq" $C1

echo
echo "passed: $pass  failed: $fail"
[ $fail -eq 0 ]
