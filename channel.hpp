/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nayache <nayache@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/06/10 14:32:28 by nayache           #+#    #+#             */
/*   Updated: 2022/06/24 18:22:14 by nayache          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include <vector>
# include "server.hpp"

class Channel
{
	public:


    Channel(std::string name) : _name(name), _topic("Default Topic"){}
		~Channel() {}
			
		void addUser(User* x)
		{
			std::stringstream stream;

			this->_users.push_back(x);
			x->setChannel(this->_name);
			
			if (this->_users.size() == 1) // si je suis le 1er du channel je suis le OPS
				this->_users[0]->setOperator(true);
			else
				x->setOperator(false);
			
			std::string response;
			if (this->_users.size() == 1)
			{
				//-----JOIN---
				response = ":" + x->getId() + " JOIN " + this->_name + "\r\n";
				send(x->getfd(), response.c_str(), response.length(), 0);
				
				//----RPL_TOPIC/RPL_NOTOPIC------
				response = ":42-IRC 332 " + x->getNickName() + " " + this->_name + " :" + this->_topic + "\r\n";
				send(x->getfd(), response.c_str(), response.length(), 0);
				saveTime(); //-----> save time to creation of topic
				
				//---MODE(facultatif)---		
				response = ":42-IRC MODE " + this->_name + " +nt\r\n";
				send(x->getfd(), response.c_str(), response.length(), 0);
				
				//----RPL_NAMEREPLY------
				response = ":42-IRC 353 " + x->getNickName() + " = " + this->_name + " :";
				for (std::vector<User*>::iterator it = _users.begin(); it != _users.end(); it++)
				{
					if ((*it)->getOperator() == true)
						response += "@" + (*it)->getNickName() + " ";
					else
						response += (*it)->getNickName() + " ";
				}
				response += "\r\n";
				send(x->getfd(), response.c_str(), response.length(), 0);
				
				//----RPL_ENDNAMES------
				response = ":42-IRC 366 " + x->getNickName() + " " + this->_name + " :End on /NAMES list.\r\n";
				send(x->getfd(), response.c_str(), response.length(), 0);
			}
			else
			{
				////----JOIN-----
				response = ":" + x->getId() + " JOIN " + this->_name + "\r\n";
				sendToUsers(response, 0);
			//	send(x->getfd(), response.c_str(), response.length(), 0);
				
				//-----RPL_TOPIC/RPL_NOTOPIC-------
				response = ":42IRC 332 " + x->getNickName() + " " + this->_name + " :" + this->_topic + "\r\n";
				send(x->getfd(), response.c_str(), response.length(), 0);

				//-----RPL_WHOTIME------
				response = ":42IRC 333 " + x->getNickName() + " " + this->_name + " " + this->_users[0]->getId() + " " + this->_topicDate + "\r\n";
				std::cout << this->_topicDate << std::endl;
				send(x->getfd(), response.c_str(), response.length(), 0);

				//----RPL_NAMEREPLY------
				response = ":42IRC 353 " + x->getNickName() + " @ " + this->_name + " :";
				for (std::vector<User*>::iterator it = _users.begin(); it != _users.end(); it++)
				{
					if ((*it)->getOperator() == true)
						response += "@" + (*it)->getNickName() + " ";
					else
						response += (*it)->getNickName() + " ";
				}
				response += "\r\n";
				send(x->getfd(), response.c_str(), response.length(), 0);
				
				//----RPL_ENDNAMES------
				response = ":42-IRC 366 " + x->getNickName() + " " + this->_name + " :End on /NAMES list.\r\n";
				send(x->getfd(), response.c_str(), response.length(), 0);
			}
		}
		
		void deleteUser(User* x, std::string msg)
		{
			std::vector<User*>::iterator idxUser = userExist(x->getUserName());
			if (idxUser == this->_users.end())
				return;
			
			this->_users.erase(idxUser);
		}

		std::vector<User*>::iterator userExist(std::string userName)
		{
			for (std::vector<User*>::iterator it = _users.begin(); it != _users.end(); it++)
			{
				if (userName == (*it)->getUserName())
					return (it);
			}
			return (this->_users.end());
		}

		void sendToUsers(std::string msg, User* without) {

			for (std::vector<User*>::iterator it = _users.begin(); it != _users.end(); it++)
			{
				if (*it != without)
					send((*it)->getfd(), msg.c_str(), msg.length(), 0);
			}
		}
		
		void saveTime()
		{
			time_t now = time(NULL);
			tm *gmtm = gmtime(&now);
			std::stringstream tt;
			tt << now;
			this->_topicDate = tt.str();
		}

		std::string	getName() const
		{
			return (this->_name);
		}

	private:

		std::string			_name;
		std::string			_topic;
		std::string			_topicDate;
		std::vector<User*>	_users;

};

#endif
