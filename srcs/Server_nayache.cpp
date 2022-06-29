// ************************************************************************** //
//                                                                            //
//                                                        :::      ::::::::   //
//   Server.cpp                                         :+:      :+:    :+:   //
//                                                    +:+ +:+         +:+     //
//   By: jiglesia <jiglesia@student.42.fr>          +#+  +:+       +#+        //
//                                                +#+#+#+#+#+   +#+           //
//   Created: 2022/06/14 10:51:42 by jiglesia          #+#    #+#             //
//   Updated: 2022/06/17 18:02:19 by jiglesia         ###   ########.fr       //
//                                                                            //
// ************************************************************************** //

#include "server.hpp"

void Server::printChannels(int userIndex){
	std::stringstream ss;

	for (std::vector<Channel>::iterator it = this->_channel.begin(); it != this->_channel.end(); it++){
		ss << ":" << it->getName() << "\r\n";
		break;
	}

	ss << ":End of Channel list\r\n";
	std::string response = ss.str();
	int bytes_sent = send(this->_users[userIndex].getfd(), response.c_str(), response.length(), 0);
	if (bytes_sent < 0){
		std::cerr << "could not send channels list\n";
		return ;
	}
}

bool Server::allowChannelName(std::string name){
	if (name[0] != '&' && name[0] != '#')
		return (false);
	if (name.size() == 2)
		return (name[1] >= '0' && name[1] <= '9');

	return ((name[1] >= 'a' && name[1] <= 'z') || (name[1] >= 'A' && name[1] <= 'Z'));
}


int Server::findChannel(std::string name){
	int i = 0;
	for (std::vector<Channel>::iterator it = this->_channel.begin(); it != this->_channel.end(); it++){
		if (it->getName() == name)
			return (i);
		i++;
	}
	return (-1);
}

std::string Server::getNameChannel(std::string buf){
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

void Server::getNames(std::string buffer, int index){
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

int	Server::getIndexChannel(std::string name){
		int i = 0;
		for (std::vector<Channel>::iterator it = this->_channel.begin(); it != this->_channel.end(); it++){
			if (name == it->getName())
				return (i);
			i++;
		}
		return (-1);
	}

void Server::leaveChannel(User* x, std::string msg){
	std::string nameChannel = x->getChannel();
	this->_channel[findChannel(nameChannel)].deleteUser(x, msg);
	x->setChannel("");

	std::stringstream ss;
	ss << "You are leaving " << nameChannel << " !" << std::endl;
	std::string out = ss.str();
	int bytes_sent = send(x->getfd(), out.c_str(), out.length(), 0);
	if (bytes_sent < 0)
		std::cerr << "Could not send leavechannel msg" << std::endl;
}

void Server::joinChannel(User* x, int indexChannel){
	std::string nameChannel = this->_channel[indexChannel].getName();
	if (nameChannel == x->getChannel())
	{
		std::stringstream ss;
		ss << "You are already in " << nameChannel << " channel!" << std::endl;
		std::string out = ss.str();
		int bytes_sent = send(x->getfd(), out.c_str(), out.length(), 0);
		if (bytes_sent < 0)
			std::cerr << "Could not send" << std::endl;
		return;
	}

	leaveChannel(x, "");
	this->_channel[indexChannel].addUser(x);
}

bool	Server::occurName(User& x){
	for (std::vector<User>::iterator it = this->_users.begin(); it != this->_users.end(); it++)
	{
		if (x.getUserName() == it->getUserName())
			if (x.getPort() != it->getPort())
				return (true);
	}
	return (false);
}
