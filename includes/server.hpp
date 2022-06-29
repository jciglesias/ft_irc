// ************************************************************************** //
//                                                                            //
//                                                        :::      ::::::::   //
/*   server.hpp                                         :+:      :+:    :+:   */
//                                                    +:+ +:+         +:+     //
//   By: jiglesia <jiglesia@student.42.fr>          +#+  +:+       +#+        //
//                                                +#+#+#+#+#+   +#+           //
//   Created: 2022/06/08 16:28:11 by jiglesia          #+#    #+#             //
//   Updated: 2022/06/17 15:08:21 by jiglesia         ###   ########.fr       //
//                                                                            //
// ************************************************************************** //

#ifndef SERVER_HPP
# define SERVER_HPP

# include <vector>
# include <iostream>
//# include <stdlib.h>
//# include <sys/socket.h>
//# include <arpa/inet.h>
# include <cstring>
# include <sstream>
//# include <exception>
# include <unistd.h>
# include "user.hpp"
# include "channel.hpp"

# define GREEN "\e[32m"
# define RESET "\e[0m"

# define BUFFERLEN 1024

class Server{
private:
	std::vector<User>	_users;
	int					_port;
	int					_sock;
	struct pollfd		_pfd;
	int					_bind_value;
	struct sockaddr_in	_address;
	int 				_listen_value;
    char				_buffer[BUFFERLEN];
	std::string			client_ip;
	std::vector<Channel>	_channel;

public:
	Server(int port, int sock);

	~Server(){}
	void addUser();
	void removeUser(int indexUser);
	std::string getline(int fd);
	int checkfd(int i);
	int checkCmd(std::string line, int i);
	int run();
/*
** Channel functions
*/
	void printChannels(int userIndex);
	bool allowChannelName(std::string name);
	int findChannel(std::string name);
	std::string getNameChannel(std::string buf);
	void getNames(std::string buffer, int index);
	int	getIndexChannel(std::string name);
	void leaveChannel(User* x, std::string msg);
	void joinChannel(User* x, int indexChannel);
	bool occurName(User& x);
};

#endif
