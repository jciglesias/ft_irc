/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nayache <nayache@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/06/10 14:32:28 by nayache           #+#    #+#             */
/*   Updated: 2022/06/10 15:55:14 by nayache          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include <vector>

class Channel
{
	public:

		Channel(std::string name, int fd) : _name(name), _fd(fd) {}
		~Channel() {}
		
		void addUser(User x)
		{
			std::cout << "user added :" << x.getNickName() << std::endl; //
			
			std::stringstream stream;
			stream << "Hello client at " << x.getIP() << ":" << x.getPort() << ". Your message was: \n" << std::string("WELCOME") + "\n";
			std::string response = stream.str();
			int bytes_sent = send(x.getfd(), response.c_str(), response.length(), 0);
			if (bytes_sent < 0) {
				std::cerr << "Could not send" << std::endl;
				return;
			}
			
			this->_users.push_back(x);
		}

		void print(std::string msg) {
			//std::cout << GREEN << _users[i].getUserName() << " : " << RESET << _buffer << std::endl;

			for (std::vector<User>::iterator it = _users.begin(); it != _users.end(); it++) {
				int bytes_sent = send(it->getfd(), msg.c_str(), msg.length(), 0);
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
		std::vector<User>	_users;
		int					_size;
		int					_fd;

};

#endif
