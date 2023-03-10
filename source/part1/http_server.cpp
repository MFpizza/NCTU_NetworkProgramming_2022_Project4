#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <sstream>
#include <string.h>
#include <sys/wait.h>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>

using boost::asio::ip::tcp;
using boost::asio::io_service;
using std::cout;
using std::endl;
using std::cerr;
boost::asio::io_context io_context;

class session
  : public std::enable_shared_from_this<session>
{
public:
  session(tcp::socket socket)
    : socket_(std::move(socket))
  {
  }

  void start()
  {
    do_read();
  }

private:
  void do_read()
  {
    auto self(shared_from_this());
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
        [this, self](boost::system::error_code ec, std::size_t length)
        {
          if (!ec)
          {
            do_write(length);
          }
        });
  }
  char status_str[200] = "HTTP/1.1 200 OK\n";
   char REQUEST_METHOD[100];
            char REQUEST_URI[1024];
            char SERVER_PROTOCOL[100];
            char HTTP_HOST[100];
            char SERVER_ADDR[100];
            char SERVER_PORT[10];
            char REMOTE_ADDR[100];
            char REMOTE_PORT[10];
            char EXEC_FILE[100]="\0";
            char QUERY_STRING[1024]="\0";
             std::string target_uri;
             
  char status_forbidden[200] = "HTTP/1.1 403 Forbidden\n";
  void do_write(std::size_t length)
  {
    auto self(shared_from_this());
    //* important
    //* REQUEST_METHOD, REQUEST_URI, SERVER_PROTOCOL, HTTP_HOST
           
						sscanf(data_,"%s %s %s\n\rHost: %s\n",
						REQUEST_METHOD,REQUEST_URI,SERVER_PROTOCOL,HTTP_HOST);

            //* SERVER_ADDR, SERVER_PORT, REMOTE_ADDR, REMOTE_PORT
            
            strcpy(SERVER_ADDR,
                 socket_.local_endpoint().address().to_string().c_str());
            sprintf(SERVER_PORT, "%u", socket_.local_endpoint().port());
            strcpy(REMOTE_ADDR,
                  socket_.remote_endpoint().address().to_string().c_str());
            sprintf(REMOTE_PORT, "%u", socket_.remote_endpoint().port());
            //* QUERY STRING
            
            sscanf(REQUEST_URI,"%[^?]?%s",EXEC_FILE,QUERY_STRING);
            EXEC_FILE[strlen(EXEC_FILE)]='\0';
            QUERY_STRING[strlen(QUERY_STRING)]='\0';
            target_uri = std::string(EXEC_FILE);
						target_uri = "."+target_uri;

    // if(target_uri!="./panel.cgi"){
    //   boost::asio::async_write(socket_, boost::asio::buffer(status_forbidden, strlen(status_forbidden)),
    //     [this, self](boost::system::error_code ec, std::size_t /*length*/)
    //     {
    //       if (!ec)
    //       {
    //         cout<<"forbidden"<<endl;
    //       }
    //       else{
    //         cout<<"ERROR!\n";
    //         cout<<ec<<endl;
    //       }
    //     });
    // }
    // else
    boost::asio::async_write(socket_, boost::asio::buffer(status_str, strlen(status_str)),
        [this, self](boost::system::error_code ec, std::size_t /*length*/)
        {
          if (!ec)
          {
            // cout<<data_<<endl;

            
            
            
            // cout<<"\nRU: "<<REQUEST_URI<<endl;
            // cout<<"EXEC: "<<EXEC_FILE<<endl;
            // cout<<"QS: "<<QUERY_STRING<<endl;
            // cout<<"TU: "<<target_uri<<endl;
    
            
              setenv("REQUEST_METHOD", REQUEST_METHOD, 1);
              setenv("REQUEST_URI", REQUEST_URI, 1);
              setenv("QUERY_STRING", QUERY_STRING, 1);
              setenv("SERVER_PROTOCOL", SERVER_PROTOCOL, 1);
              setenv("HTTP_HOST", HTTP_HOST, 1);
              setenv("SERVER_ADDR", SERVER_ADDR, 1);
              setenv("SERVER_PORT", SERVER_PORT, 1);
              setenv("REMOTE_ADDR", REMOTE_ADDR, 1);
              setenv("REMOTE_PORT", REMOTE_PORT, 1);
              setenv("EXEC_FILE", EXEC_FILE, 1);

              int sock = socket_.native_handle();
							dup2(sock, STDIN_FILENO);
							dup2(sock, STDOUT_FILENO);
							socket_.close();
              // if(target_uri !="./panel.cgi"){
                  
              // }
              if (execlp(target_uri.c_str(), target_uri.c_str(), NULL) < 0) {
                // std::cout << "Content-type:text/html\r\n\r\n<h1>FAIL</h1>";
                // cout<<"fuck out"<<endl;
                cerr<<"finished\n";
              
            }
            do_read();
          }
          else{
            cout<<"ERROR!\n";
            cout<<ec<<endl;
          }
        });
  }

  tcp::socket socket_;
  enum { max_length = 1024 };
  char data_[max_length];
};

class server
{
public:
  server(boost::asio::io_context& io_context, short port)
    : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)),
      signal_(io_context, SIGCHLD)
  {
    cerr<<"~~~~~server start"<<endl;
    start_signal_wait();
    do_accept();
  }
  ~server(){
    cerr<<"pid: "<<pid<<" server end~~~~~"<<endl;
  }

private:

  void start_signal_wait()
  {
    signal_.async_wait(boost::bind(&server::handle_signal_wait, this));
  }

  void handle_signal_wait()
  {
    // Only the parent process should check for this signal. We can determine
    // whether we are in the parent by checking if the acceptor is still open.
    if (acceptor_.is_open())
    {
      // Reap completed child processes so that we don't end up with zombies.
      int status = 0;
      while (waitpid(-1, &status, WNOHANG) > 0) {}

      start_signal_wait();
    }
  }

  void do_accept()
  {
    acceptor_.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket)
        {
          if (!ec)
          {
            cerr<<"ready to fork"<<endl;
            io_context.notify_fork(io_service::fork_prepare);
            pid = fork();
            if (pid == 0)
            {
                io_context.notify_fork(io_service::fork_child);
                acceptor_.close();
                signal_.cancel();
                std::make_shared<session>(std::move(socket))->start(); 
                cerr<<"session create finish"<<endl;    
            }
            else
            {

              io_context.notify_fork(io_service::fork_parent);

              socket.close();
              do_accept();
            }
          }

        });
      int count = 0;
      cout<<"end accept"<<endl;
  }
  pid_t pid;
  boost::asio::signal_set signal_;
  tcp::acceptor acceptor_;
};

int main(int argc, char* argv[])
{
  try
  {
    if (argc != 2)
    {
      std::cerr << "Usage: async_tcp_echo_server <port>\n";
      return 1;
    }

    cout<<"Port: "<<std::atoi(argv[1])<<endl;
    server s(io_context, std::atoi(argv[1]));

    io_context.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0; 
}
