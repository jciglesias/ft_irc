// ************************************************************************** //
//                                                                            //
//                                                        :::      ::::::::   //
/*   server.hpp                                         :+:      :+:    :+:   */
//                                                    +:+ +:+         +:+     //
//   By: jiglesia <jiglesia@student.42.fr>          +#+  +:+       +#+        //
//                                                +#+#+#+#+#+   +#+           //
//   Created: 2022/06/08 16:28:11 by jiglesia          #+#    #+#             //
/*   Updated: 2022/06/30 16:02:53 by nayache          ###   ########.fr       */
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
	void list(int idx){
		std::string response;
		response = ":42-IRC 321 " + this->_users[idx].getNickName() + " Channel :Users Name\r\n";
		send(this->_users[idx].getfd(), response.c_str(), response.length(), 0);

		for (std::vector<Channel>::iterator it = this->_channel.begin(); it != this->_channel.end(); it++)
		{
			response = ":42-IRC 322 " + this->_users[idx].getNickName() + " " + it->getName() + " " + it->getSize() + " :" + it->getTopic() + "\r\n"; 
			send(this->_users[idx].getfd(), response.c_str(), response.length(), 0);
		}
		response = ":42-IRC 323 " + this->_users[idx].getNickName() + " :End of LIST\r\n"; 
		send(this->_users[idx].getfd(), response.c_str(), response.length(), 0);
	}
	
	User* getUser(std::string nick)
	{
		for (std::vector<User>::iterator it = this->_users.begin(); it != this->_users.end(); it++)
		{
			if (it->getNickName() == nick)
				return (&(*it));
		}
		return (NULL);
	}

	void sendMsg(User* x, std::string nick, std::string msg)
	{
		User* target;

		target = getUser(nick);
		if (target == NULL)
			return;

		std::string response = ":" + x->getId() + " PRIVMSG " + nick + " :" + msg + "\r\n";
		std::cout << response << std::endl;
		send(target->getfd(), response.c_str(), response.length(), 0);
	}

	bool allowChannelName(std::string name){
		if (name[0] != '&' && name[0] != '#')
			return (false);
		if (name.size() == 2)
			return (name[1] >= '0' && name[1] <= '9');
		
		return ((name[1] >= 'a' && name[1] <= 'z') || (name[1] >= 'A' && name[1] <= 'Z'));
	}

	int findChannel(std::string name){
		int i = 0;
		for (std::vector<Channel>::iterator it = this->_channel.begin(); it != this->_channel.end(); it++){
			if (it->getName() == name)
				return (i);
			i++;
		}
		return (-1);
	}

	std::string getNameChannel(std::string buf){
		char*	str = const_cast<char *>(buf.c_str());
		char *delimit = strchr(str + 5, ' ');
		if (delimit != NULL)
			*delimit = '\0';
		else{
			char *delimit = strchr(str + 5, '\r');
			if (delimit != NULL)
				*delimit = '\0';
		}
		
		for (int i = 0; i < 5; i++)
			str++;

		std::string tmp(str);
		std::string name = tmp.c_str();

		return (name);
	}

	void getNames(std::string buffer, int index) {
		char*	str = const_cast<char *>(buffer.c_str());
		if (!strncmp(str, "CAP LS 302", 10)){
			std::string	nickName;
			std::string	userName;

			char* delimit = strchr(str + 17, '\n');
			delimit[-1] = '\0';
			nickName = str + 17;

			delimit = strstr(delimit + 1, "USER ");
			char* str = strstr(delimit + 6, " 0 * :");
			*str = '\0';
			userName = delimit + 5;

			this->_users[index].setName(nickName, userName);
		}
	}
	
	int	getIndexChannel(std::string name){
		int i = 0;
		for (std::vector<Channel>::iterator it = this->_channel.begin(); it != this->_channel.end(); it++){
			if (name == it->getName())
				return (i);
			i++;
		}
		return (-1);
	}

	void joinChannel(User* x, int indexChannel)
	{
		std::string nameChannel = this->_channel[indexChannel].getName();
		if (this->_channel[indexChannel].userExist(x->getNickName()) == true)
		{
			std::string ss;
			ss = "\002You are already in " + nameChannel + "\r\n";
			int bytes_sent = send(x->getfd(), ss.c_str(), ss.length(), 0);
			if (bytes_sent < 0)
				std::cerr << "Could not send" << std::endl;
			return;
		}
		this->_channel[indexChannel].addUser(x);
	}

	Server(int port, int sock) : _port(port), _sock(sock){
		_pfd.fd = _sock;
		_pfd.events = POLLIN;
		memset(&_address, 0, sizeof(_address));
		_address.sin_family = AF_INET;
		_address.sin_port = htons(_port);
		_bind_value = bind(_sock, (struct sockaddr *)&_address, sizeof(_address));
		if (_bind_value < 0){
			std::cout << "could not bind\n";
			throw std::exception();
		}
		_listen_value = listen(_sock, 1);
		if (_listen_value < 0)
			throw std::exception();
	}
	
	bool	occurName(User& x){
		for (std::vector<User>::iterator it = this->_users.begin(); it != this->_users.end(); it++)
		{
			if (x.getNickName() == it->getNickName())
				if (x.getPort() != it->getPort())
					return (true);
		}
		return (false);
	}
	
	~Server(){}
	
	void addUser(){
		try {
			_users.push_back(User(_sock));
			this->_channel[0].addUser(&(this->_users.back()));
		}
		catch (std::exception &e){
			std::cerr << "Could not accept user" << std::endl;
		}
		std::cout << "Accepted new client @ " << _users.back().getIP() << ":" << _users.back().getPort() << std::endl;
	}
	
	void removeUser(int indexUser)
	{
		std::string channel = this->_users[indexUser].getChannel();

		std::vector<User>::iterator pos(&(this->_users[indexUser]));
		this->_users.erase(pos);
	}

	std::string getline(int fd){
		std::stringstream stream;
		memset(_buffer, 0, BUFFERLEN);
		int bytes_read = read(fd, _buffer, BUFFERLEN - 1);
		if (_buffer[bytes_read - 1] == '\n')
			_buffer[bytes_read - 1] = 0;
		stream << _buffer;
		return stream.str();
	}

	int checkfd(int i){
		std::string str = this->getline(_users[i].getfd());
		std::cout << str << std::endl; // juste pour voir les envoi du client
		// 5 recv
 		if (_users[i].getFirstMsg() == false)
		{
			getNames(_buffer, i);
			if (occurName(this->_users[i]) == true)
				removeUser(i);
			else
			{
				std::string msg = ":127.0.0.1 001 " + this->_users[i].getNickName() + " :\002Welcome to the Internet Relay Network\r\n";
				int bytes_sent = send(this->_users[i].getfd(), msg.c_str(), msg.length(), 0);
			}
		}
  		else {
			if (_buffer[0] == 'Q' && _buffer[1] == 'U' && _buffer[2] == 'I' && _buffer[3] == 'T'){
  			std::cout << "Client at " << _users[i].getIP() << ":" << _users[i].getPort() << " has disconnected." << std::endl;
				_users.erase(_users.begin() + i);
				if (_users.size() == 0){
					std::cout << "Shutting down socket." << std::endl;
					return 0;
				}
			}
			else if (!strncmp(_buffer, "LIST", 4)){
				list(i);
			}
			else if (!strncmp(_buffer, "JOIN #", 6))
			{
				std::string nameChannel(getNameChannel(_buffer));
				if (allowChannelName(nameChannel) == true){
					int nbChannel = findChannel(nameChannel);
					if (nbChannel < 0){
						this->_channel.push_back(nameChannel);
						joinChannel(&(this->_users[i]), this->_channel.size() - 1);
					}
					else
						joinChannel(&(this->_users[i]), nbChannel);
				}
			}
			else if (!strncmp(_buffer, "PART #", 6)) //quitter un channel
			{
				char* str = const_cast<char *>(_buffer);
				char* delimit  = strchr(str + 5, ' ');
				*delimit = '\0';

				std::string channel(str + 5);
				int idx = getIndexChannel(channel);
				if (idx > -1)
				{
					char *end = strchr(delimit + 1, ':');
					str = strchr(end + 1, '\r');
					*str = '\0';
					std::string msg(end + 1);
					this->_channel[idx].deleteUser(msg, &(this->_users[i]));
				}
			}
			else
			{
				if (!strncmp(_buffer, "PRIVMSG ", 8)) // a modifier(essayer de split les args et voir si erreur de format msg) /msg <nick> <:text>
				{
					if (!strncmp(_buffer, "PRIVMSG #", 9)) //msg to channel
					{
						char* str = const_cast<char *>(_buffer);
						char* delimit  = strchr(str + 8, ' ');
						*delimit = '\0';

						delimit = strchr(delimit + 1, ':');
						*delimit = '\0';
		  				std::string channel(str + 8);
						int idx = getIndexChannel(channel);
						if (idx > -1)
						{
							char *end = strchr(delimit + 1, '\r');
							*end = '\0';
							std::string msg(delimit + 1);
							this->_channel[idx].privMsg(msg, &(this->_users[i]));
						}
					}
					else //msg to user
					{
						char* str = const_cast<char *>(_buffer);
						char* delimit  = strchr(str + 8, ' ');
						if (delimit == NULL)
							std::cerr << "error format PRIVMSG\n";
						*delimit = '\0';
						
						std::string nick(str + 8);
						str = strchr(delimit + 1, ':');
						delimit = strchr(str + 1, '\r');
						*delimit = '\0';
						if (delimit == NULL)
							std::cerr << "error format PRIVMSG\n";
						std::string msg(str + 1);
						sendMsg(&(this->_users[i]), nick, msg);
					}
				}
			}
   	}
		return 1;
	}

	int run(){
		while (1) {
			poll(&_pfd, 1, 100);
			if (_pfd.revents == POLLIN){
				try {
					_users.push_back(User(_sock));
				}
				catch (std::exception &e){
					std::cerr << "Could not accept user" << std::endl;
				}
				std::cout << "Accepted new client @ " << _users.back().getIP() << ":" << _users.back().getPort() << std::endl;
			}
			else{
				for (unsigned long i = 0; i < _users.size(); i++){
					_users[i].u_poll(100);
					if (_users[i].getRevents() == POLLIN){
						if (!this->checkfd(i))
							return 0;
					}
				}
			}
		}
	}
};

#endif
