// ************************************************************************** //
//                                                                            //
//                                                        :::      ::::::::   //
//   main.cpp                                           :+:      :+:    :+:   //
//                                                    +:+ +:+         +:+     //
//   By: jiglesia <jiglesia@student.42.fr>          +#+  +:+       +#+        //
//                                                +#+#+#+#+#+   +#+           //
//   Created: 2022/06/08 17:37:51 by jiglesia          #+#    #+#             //
//   Updated: 2022/06/09 10:42:43 by jiglesia         ###   ########.fr       //
//                                                                            //
// ************************************************************************** //

#include "server.hpp"
#include "user.hpp"

int main(int ac, char **av) {
    if (ac < 2) {
        std::cout << "Usage: " << av[0] << " <port number>" << std::endl;
        return 0;
    }
	try {
		Server s(atoi(av[1]), socket(AF_INET, SOCK_STREAM, 0));
		std::cout << "waiting for client\n";
		s.run();
	}
	catch (std::exception &e) {
		return 1;
	}
	return 0;
}
