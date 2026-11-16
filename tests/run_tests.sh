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
			--track-fds=yes --error-exitcode=99 "$@"
	else
		"$@"
	fi
}

ok()   { pass=$((pass + 1)); printf '  \033[32mPASS\033[0m %s\n' "$1"; }
ko()   { fail=$((fail + 1)); printf '  \033[31mFAIL\033[0m %s\n' "$1"; [ -n "${2:-}" ] && printf '       %s\n' "$2"; }

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

echo "== gameplay (scripted input, dummy video driver)"
play() { # name expected_status_substring args...
	local name=$1 expect=$2; shift 2
	out=$(run "$BIN" "$@" 2>&1); rc=$?
	if [ $rc -eq 0 ] && grep -q -- "$expect" <<<"$out"; then ok "$name"
	else ko "$name" "rc=$rc $(tail -n 3 <<<"$out")"; fi
}
L1=assets/maps/level1.ber
L2=assets/maps/level2.ber
play "walls block, moves not counted" "moves=0" --keys "aaaa" --frames 10 tests/maps/valid/min.ber
play "collect + exit clears level"   "state=COMPLETE moves=9 coins=2/2" --keys "swwddddss" --frames 20 $L1
play "exit closed until all coins"   "state=PLAYING" --keys "sddddd" --frames 10 $L1
play "campaign advances on ENTER"    "level=2 state=PLAYING" --keys "swwddddsse" --frames 20
play "walking into enemy = game over" "state=GAME_OVER" --keys "dddd" --frames 10 $L2
play "restart resets attempt"        "state=PLAYING moves=0" --keys "ddddr" --frames 10 $L2
# Enemy reaches the idle player after 7 steps * 0.45s = 3.15s of game time,
# regardless of frame rate: same outcome at 60 Hz and 240 Hz.
play "enemy idle at 2.0s @60Hz"      "state=PLAYING" --dt 0.016666 --frames 120 $L2
play "enemy idle at 2.0s @240Hz"     "state=PLAYING" --dt 0.0041666 --frames 480 $L2
play "enemy hits at 3.5s @60Hz"      "state=GAME_OVER" --dt 0.016666 --frames 210 $L2
play "enemy hits at 3.5s @240Hz"     "state=GAME_OVER" --dt 0.0041666 --frames 840 $L2
play "ESC/quit exits cleanly"        "level=1" --keys "dq" $L1
play "multi-map custom campaign"     "level=2 state=PLAYING" --keys "swwddddsse" --frames 20 $L1 $L2

echo
echo "passed: $pass  failed: $fail"
[ $fail -eq 0 ]
