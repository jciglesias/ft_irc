/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nayache <nayache@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/06/10 14:32:28 by nayache           #+#    #+#             */
//   Updated: 2022/06/17 16:30:15 by jiglesia         ###   ########.fr       //
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include <vector>
//# include "server.hpp"

class Channel
{
	public:


    Channel(std::string name) : _name(name) {}
	~Channel() {}

	void addUser(User* x){
		std::stringstream stream;

		this->_users.push_back(x);
		x->setChannel(this->_name);

		if (this->_users.size() == 1) // si je suis le 1er du channel je suis le OPS
			this->_users[0]->setOperator(true);
		else
			x->setOperator(false);

		std::string response;
		if (x->getNickName() == "dan"){
			response = ":" + x->getNickName() + "!~" + x->getNickName() + "@" + x->getIP() + " JOIN " + this->_name + "\r\n"
				+ ":42-IRC-SERVER MODE " + this->_name + " +nt\r\n"
				+ ":42-IRC-SERVER 353 " + x->getNickName() + " = " + this->_name + " :@" + x->getNickName() + "\r\n";
			+ ":42-IRC-SERVER 366 " + x->getNickName() + " " + this->_name + " :End on /NAMES list.\r\n";
			std::cout << response;
		}
		else{
			std::cout << "ICI\n";
			response = ":" + x->getNickName() + "!~" + x->getNickName() + "@" + x->getIP() + " JOIN " + this->_name + "\r\n"
				+ ":42-IRC-SERVER 332 " + x->getNickName() + " " + this->_name + " :" + this->_topic + "\r\n"
				+ ":42-IRC-SERVER 333 " + x->getNickName() + " " + this->_name + " " + this->_users[0]->getNickName() + "!~" + this->_users[0]->getNickName() + "@" + this->_users[0]->getIP() + "\r\n"
				+ ":42-IRC-SERVER 353 " + x->getNickName() + " @ " + this->_name + " :" + x->getNickName() + " @" + this->_users[0]->getNickName() + "\r\n";
			+ ":42-IRC-SERVER 366 " + x->getNickName() + " " + this->_name + " :End on /NAMES list.\r\n";
			std::cout << response;
		}

		int bytes_sent = send(x->getfd(), response.c_str(), response.length(), 0);
		if (bytes_sent < 0)
			std::cerr << "Could not sent\n";
	}

	void deleteUser(User* x, std::string msg){
		std::vector<User*>::iterator idxUser = userExist(x->getUserName());
		if (idxUser == this->_users.end())
			return;

		this->_users.erase(idxUser);

		std::string finalMsg;
		if (msg != "")
			finalMsg = x->getUserName() + " leave channel \"" + msg + "\"\n";
		else
			finalMsg = x->getUserName() + " leave channel\n";

		print(finalMsg);
	}

	std::vector<User*>::iterator userExist(std::string userName){
		for (std::vector<User*>::iterator it = _users.begin(); it != _users.end(); it++)
		{
			if (userName == (*it)->getUserName())
				return (it);
		}
		return (this->_users.end());
	}

	void print(std::string msg) {
		(void)msg;
		for (std::vector<User*>::iterator it = _users.begin(); it != _users.end(); it++) {
			std::string imsg = "332";
			int bytes_sent = send((*it)->getfd(), imsg.c_str(), imsg.length(), 0);
			if (bytes_sent < 0) {
				std::cerr << "Could not send" << std::endl;
				return;
			}
		}
	}

	std::string	getName() const{
		return (this->_name);
	}

private:

	std::string			_name;
	std::string			_topic;
	std::vector<User*>	_users;

};

#endif
