/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nayache <nayache@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/06/10 14:32:28 by nayache           #+#    #+#             */
/*   Updated: 2022/06/30 15:57:29 by nayache          ###   ########.fr       */
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
			
			if (this->_users.size() == 1) // si je suis le 1er du channel je suis OPS
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
		
		void deleteUser(std::string msg, User* x)
		{
			std::string response;
			std::vector<User*>::iterator it = getPosition(x->getNickName());
			if (it == this->_users.end())
			{
				response = ":42-IRC 442 " + x->getNickName() + " " + this->_name + " :You are not on tchat channel\r\n";
				send(x->getfd(), response.c_str(), response.length(), 0);
				return;
			}
			response = ":" + x->getId() + " PART " + this->_name;
			if (msg != "Leaving")
				response += " :" + msg;
			response += "\r\n";

			//send(x->getfd(), response.c_str(), response.length(), 0);
			sendToUsers(response, 0);
			this->_users.erase(it);
		}

		bool userExist(std::string nickName)
		{
			for (std::vector<User*>::iterator it = _users.begin(); it != _users.end(); it++)
			{
				if (nickName == (*it)->getNickName())
					return (true);
			}
			return (false);
		}
		
		std::vector<User*>::iterator getPosition(std::string nickName)
		{
			for (std::vector<User*>::iterator it = _users.begin(); it != _users.end(); it++)
			{
				if (nickName == (*it)->getNickName())
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

		void privMsg(std::string msg, User* client)
		{
			std::string response = ":" + client->getId() + " PRIVMSG " + this->_name + " :" + msg + "\r\n";
			std::cout << response << std::endl;
			sendToUsers(response, client);
		}
		
		void saveTime()
		{
			time_t now = time(NULL);
			tm *gmtm = gmtime(&now);
			std::stringstream tt;
			tt << now;
			this->_topicDate = tt.str();
		}

		std::string	getName() const { return (this->_name); }
		std::string	getTopic() const { return (this->_topic); }
		std::string getSize() const
		{
			std::stringstream ss;
			ss << this->_users.size();
			return (ss.str());
		}

	private:

		std::string			_name;
		std::string			_topic;
		std::string			_topicDate;
		std::vector<User*>	_users;

};

#endif
