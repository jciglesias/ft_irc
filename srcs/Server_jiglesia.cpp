// ************************************************************************** //
//                                                                            //
//                                                        :::      ::::::::   //
//   Server_jiglesia.cpp                                :+:      :+:    :+:   //
//                                                    +:+ +:+         +:+     //
//   By: jiglesia <jiglesia@student.42.fr>          +#+  +:+       +#+        //
//                                                +#+#+#+#+#+   +#+           //
//   Created: 2022/06/14 11:08:20 by jiglesia          #+#    #+#             //
//   Updated: 2022/06/17 15:48:16 by jiglesia         ###   ########.fr       //
//                                                                            //
// ************************************************************************** //

#include "server.hpp"

Server::Server(int port, int sock) : _port(port), _sock(sock){
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

	Channel	Default("Default");
	this->_channel.push_back(Default);
}

void Server::removeUser(int indexUser){
	std::string channel = this->_users[indexUser].getChannel();

	leaveChannel(&(this->_users[indexUser]), "");
	std::cout << "Client at " << this->_users[indexUser].getIP() << ":" << this->_users[indexUser].getPort() << " has disconnected." << std::endl;
	std::vector<User>::iterator pos(&(this->_users[indexUser]));
	this->_users.erase(pos);
}

void Server::addUser(){
	try {
		_users.push_back(User(_sock));
		this->_channel[0].addUser(&this->_users.back());
	}
	catch (std::exception &e){
		std::cerr << "Could not accept user" << std::endl;
	}
	std::cout << "Accepted new client @ " << _users.back().getIP() << ":" << _users.back().getPort() << std::endl;
}

std::string Server::getline(int fd){
	std::stringstream stream;
	memset(_buffer, 0, BUFFERLEN);
	int bytes_read = read(fd, _buffer, BUFFERLEN - 1);
	if (_buffer[bytes_read - 1] == '\n')
		_buffer[bytes_read - 1] = 0;
	stream << _buffer;
	return stream.str();
}

int Server::checkCmd(std::string line, int i){
	if (!line.compare(0, 4, "QUIT")){
		std::cout << "Client at " << _users[i].getIP() << ":" << _users[i].getPort() << " has disconnected." << std::endl;
		_users.erase(_users.begin() + i);
		if (_users.size() == 0){
			std::cout << "Shutting down socket." << std::endl;
			return 0;
		}
	}
	else if (!line.compare(0, 4, "LIST")){
		printChannels(i);
	}
	else if (!line.compare(0, 6, "JOIN #")){
		std::string nameChannel(getNameChannel(line.c_str()));
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
	else{
		std::stringstream stream;
		std::cout << GREEN << _users[i].getUserName() << "(" << _users[i].getNickName() << ") : " << RESET << line << std::endl; // to server
		if (this->_users[i].getOperator() == true)
			stream << "(Ops)";
		stream << _users[i].getUserName() << "(" << _users[i].getNickName() << ") : " << line << std::endl; // to channel
		std::string msg = stream.str();
		int idx = getIndexChannel(this->_users[i].getChannel());
		this->_channel[idx].print(msg);
	}
	return 1;
}

int Server::checkfd(int i){
	std::string str = this->getline(_users[i].getfd());
	// 5 recv
	if (_users[i].getFirstMsg() == false){
		getNames(str.c_str(), i);
		//if (occurName(this->_users[i]) == true)
		//removeUser(i);
		std::string msg = ":127.0.0.1 001 " + this->_users[i].getNickName() + " :Welcome to the Internet Relay Network\r\n";
		send(this->_users[i].getfd(), msg.c_str(), msg.length(), 0);
	}
	else {
		return checkCmd(str, i);
	}
	return 1;
}

int Server::run(){
	while (1) {
		poll(&_pfd, 1, 100);
		if (_pfd.revents == POLLIN){
			try {
				_users.push_back(User(_sock));
				//this->_channel[0].addUser(&(this->_users.back()));
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
