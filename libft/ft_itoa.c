/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_itoa.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zzaoui <zzaoui@student.1337.ma>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/26 11:54:35 by zzaoui            #+#    #+#             */
/*   Updated: 2024/11/04 14:51:55 by zzaoui           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

/**
 * ft_itoa - Allocates and returns a string reprresenting
 *		an integer received as argument.
 * @n: the integer to convert
 * Return: the string representing the integer, NULL otherwise.
 */
char	*ft_itoa(int n)
{
	char			buff[12];
	unsigned int	m;
	int				i;

	i = 0;
	if (n == 0)
		return (ft_strdup("0"));
	if (n < 0)
	{
		buff[i++] = '-';
		n = (unsigned int) -n;
	}
	m = 1;
	while (n / m >= 10)
		m *= 10;
	while (m >= 1)
	{
		buff[i++] = ((n / m) % 10) + '0';
		m /= 10;
	}
	buff[i] = '\0';
	return (ft_strdup(buff));
}
