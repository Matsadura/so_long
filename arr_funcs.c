/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   arr_funcs.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zzaoui <zzaoui@student.1337.ma>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/02 18:09:43 by zzaoui            #+#    #+#             */
/*   Updated: 2025/02/02 20:30:22 by zzaoui           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "so_long.h"

/**
 * print_2darray - Prints a 2D array
 * @arr: The 2D array
 */
void	print_2darray(char **arr)
{
	int	i;

	i = 0;
	while (arr[i])
	{
		ft_printf("%s\n", arr[i]);
		i++;
	}
}

/**
 * free_2darray - Frees a 2D dynamically allocated array
 * @arr: The 2D array
 */
void	free_2darray(char **arr)
{
	int	i;

	i = 0;
	if (!arr)
		return ;
	while (arr[i])
	{
		free(arr[i]);
		i++;
	}
	free(arr);
}
