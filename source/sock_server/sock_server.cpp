#include "sock_session.h"
class server
{
public:
  server(boost::asio::io_context& io_context, short port)
    : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)),
      signal_(io_context, SIGCHLD)
  {
    start_signal_wait();
    do_accept();
  }

private:

  void start_signal_wait()
  {
    signal_.async_wait(boost::bind(&server::handle_signal_wait, this));
  }

  void handle_signal_wait()
  {
    if (acceptor_.is_open())
    {
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
            // cerr<<"there is an accept"<<endl;
            io_context.notify_fork(io_service::fork_prepare);
            pid = fork();
            if (pid == 0)
            {
                io_context.notify_fork(io_service::fork_child);
                acceptor_.close();
                signal_.cancel();
                cerr<<"create session"<<endl;
                std::make_shared<session>(std::move(socket))->start(); 
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
  }
  pid_t pid;
  boost::asio::signal_set signal_;
  tcp::acceptor acceptor_;
};
boost::asio::io_context io_context;

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
