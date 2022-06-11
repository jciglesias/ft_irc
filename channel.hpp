/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nayache <nayache@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/06/10 14:32:28 by nayache           #+#    #+#             */
/*   Updated: 2022/06/10 22:12:02 by nayache          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include <vector>

class Channel
{
	public:


    Channel(std::string name) : _name(name) {}
		~Channel() {}
			
		void addUser(User* x)
		{
			std::stringstream stream;

			this->_users.push_back(x);
			x->setChannel(this->_name);
			if (this->_users.size() == 1)
			{
				this->_users[0]->setOperator(true);
				stream << "You are now the Operator(Ops) in this channel" << std::endl;
			}
			else
			{
				x->setOperator(false);
				std::string msg = x->getUserName() + " are join the channel\n";
				this->print(msg);
			}

			stream << "Hello client at " << x->getIP() << ":" << x->getPort() << ". Your message was: \n" << std::string("WELCOME");
			stream << " to #" << this->_name << std::endl;
			
			std::string response = stream.str();
			int bytes_sent = send(x->getfd(), response.c_str(), response.length(), 0);
			if (bytes_sent < 0) {
				std::cerr << "Could not send" << std::endl;
				return;
			}

		}
		
		void deleteUser(User* x)
		{
			std::vector<User*>::iterator idxUser = userExist(x->getUserName());
			if (idxUser == this->_users.end())
				return;
			
			this->_users.erase(idxUser);

			std::string msg = x->getUserName() + " are leave a channel\n";
			print(msg);
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

		void print(std::string msg) {
			for (std::vector<User*>::iterator it = _users.begin(); it != _users.end(); it++) {
				int bytes_sent = send((*it)->getfd(), msg.c_str(), msg.length(), 0);
				if (bytes_sent < 0) {
					std::cerr << "Could not send" << std::endl;
					return;
				}
			}
		}

		std::string	getName() const
		{
			return (this->_name);
		}

	private:

		std::string			_name;
		std::vector<User*>	_users;

};

#endif
