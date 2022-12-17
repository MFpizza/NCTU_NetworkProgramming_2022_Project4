#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <sstream>
#include <string.h>
#include <stdio.h>
#include <sys/wait.h>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>

using boost::asio::ip::tcp;
using boost::asio::io_service;
using std::cout;
using std::endl;
using std::cerr;
extern boost::asio::io_context io_context;

class session
  : public std::enable_shared_from_this<session>
{
public:
  session(tcp::socket socket)
    : socket_(std::move(socket)),http_socket(io_context),resolve(io_context)
  {
  }

  void start()
  {
    do_read();
  }

private:

  void do_read();
  bool Do_Connect();
  void printVaraible();
  void do_write(std::size_t length);
  void connectHandler(const boost::system::error_code& err);
  void Do_Relaying(int cas);

  char dip[20];
  unsigned int VN;
  unsigned int CD;
  std::string SRCIP;
  std::string SRCPORT;
  std::string DSTIP;
  std::string DSTPORT;
  std::string DOMAIN_NAME;
  unsigned char socks4_reply[200];

  tcp::socket socket_;
  tcp::socket http_socket;
  tcp::resolver resolve;
  enum { max_length = 1024 };
  unsigned char data_[max_length];
  char http_data_[max_length];
  char client_data_[max_length];
};