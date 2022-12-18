#include "sock_session.h"

void session::checkfirewall(){
    
    std::string fireWall[2];
    input = fireWall[CD-1].substr(9);
	stringstream ss1(input);
	stringstream ss2(DstIP);
	//cerr << DstIP << endl;
	string permit,dst;
	int times = 0;
	while (getline(X, permit, '.') && getline(Y,dst,'.')) { 
		//cerr << "test" << endl;
		if(permit == "*"){
			accept = true;
			break;
		}
		else if(permit == dst){
			times++;
			continue;
		}else{
			break;
		}
	}
	if(times == 4) accept = true;
}

void session::do_read()
{
    auto self(shared_from_this());
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
        [this, self](boost::system::error_code ec, std::size_t length)
        {
          if (!ec)
          {
            // cout<<"length:"<<length<<endl;
            cout<<endl;
            for(int i=0;i<length;i++){
                // if(data_[i]>='a'&&data_[i]<='z')
                //     cout<<data_[i];
                // else{
                    unsigned int temp = data_[i];
                    cout<< temp;
                // }
                cout<<"/";
            }
            cout<<endl;
            VN = data_[0];
            CD = data_[1];

            unsigned int DPort = (data_[2] << 8)| data_[3];
            DSTPORT = std::to_string(DPort);

            sprintf(dip,"%u.%u.%u.%u",data_[4],data_[5],data_[6],data_[7]);
						DSTIP = dip;

            boost::asio::ip::tcp::endpoint remote_ep = socket_.remote_endpoint();
			boost::asio::ip::address remote_ad = remote_ep.address();
			SRCIP = remote_ad.to_string();
			SRCPORT = std::to_string(remote_ep.port());

            bool getDOMAIN=false;
            for(int i=8;i<length;i++){
              if(getDOMAIN)
                DOMAIN_NAME+= data_[i];
              if(data_[i]==0)
                getDOMAIN = !getDOMAIN;
            }

            printVaraible();
            if (CD==2){
              Do_Bind();
            }
            else if(CD==1){
				bind_acceptor.close();
                Do_Connect();
            }
          }
          else{
            perror("do_read");
          }
        });
}

void session::Do_Bind(){
    auto self(shared_from_this());
    bind_acceptor.set_option(tcp::acceptor::reuse_address(true));
    bind_acceptor.listen();
	bind_port = bind_acceptor.local_endpoint().port();
    // cout<<"[bind port]:"<<bind_port<<endl;
    memset(socks4_reply, 0, sizeof(unsigned char)*200 );
    socks4_reply[0] = 0;
    socks4_reply[1] = 90;
	socks4_reply[2] = (unsigned char)(bind_port/256);
	socks4_reply[3] = (unsigned char)(bind_port%256);
    
    socket_.async_send( boost::asio::buffer(socks4_reply, 8),
				[this,self](boost::system::error_code err, std::size_t length_){
                    if(!err){
                        bind_acceptor.async_accept(http_socket, [this,self](boost::system::error_code err) {
							if (!err){
                            // cout<<"accept:\nIP:"<<http_socket.remote_endpoint().address().to_string()<<"\nPort:"<<std::to_string(http_socket.remote_endpoint().port())<<endl;
                            // cout<<"local:\nIP:"<<http_socket.local_endpoint().address().to_string()<<"\nPort:"<<std::to_string(http_socket.local_endpoint().port())<<endl;
                            
                            // Do_Relaying(3);
                            //  http_socket.async_read_some(boost::asio::buffer(data_, max_length),
                            //     [this, self](boost::system::error_code ec, std::size_t length)
                            //     {
                            //     if (!ec)
                            //     {
                            //         cout<<"length:"<<length<<endl;
                            //         cout<<endl;
                            //         for(int i=0;i<length;i++){
                            //             if(data_[i]>='a'&&data_[i]<='z')
                            //                 cout<<data_[i];
                            //             else{
                            //                 unsigned int temp = data_[i];
                            //                 cout<< temp;
                            //             }
                            //             cout<<"/";
                            //         }
                            //     }});
                                socket_.async_send( boost::asio::buffer(socks4_reply, 8),
									[this,self](boost::system::error_code err, std::size_t len){
										if(!err) {
                                            Do_Relaying(3);
										}
                                        else{
                                            perror("do bind send2");
                                        }
									});
                            }
							else{
								perror("do bind accept");
							}
						});
                    }
                    else{
                        perror("do bind send1");
                    }

                });
}

void session::Do_Connect(){
    auto self(shared_from_this());
    // has DSTIP
    if(DOMAIN_NAME.empty()){
        boost::asio::ip::tcp::endpoint dst_endpoint(boost::asio::ip::address::from_string(DSTIP),atoi(DSTPORT.c_str()));
        http_socket.async_connect(dst_endpoint, boost::bind(&session::connectHandler, self,boost::asio::placeholders::error));
    }
    // has domain name dst
    else{
        tcp::resolver::query query(DOMAIN_NAME, DSTPORT);
		// resolve.async_resolve(query,boost::bind(&session::resolveHandler, self,boost::asio::placeholders::error,boost::asio::placeholders::iterator ));
		resolve.async_resolve(query,
            [this,self](boost::system::error_code ec,tcp::resolver::iterator it){
                if(!ec){
                    boost::asio::ip::tcp::endpoint end = (*it);
				    DSTIP = end.address().to_string();
                    http_socket.async_connect(*it, boost::bind(&session::connectHandler, self,boost::asio::placeholders::error));
                }
                else{
                    perror("resolve");
                }
            }
        );
    }
}

void session::connectHandler(const boost::system::error_code& err){
    auto self(shared_from_this()); 
    memset(socks4_reply, '\0', sizeof(unsigned char)*200 );
	if(!err){
        // cout<<"connect"<<endl;
        socks4_reply[0]=0;
        socks4_reply[1]=90;
        std::string sClientIp = http_socket.remote_endpoint().address().to_string();
        unsigned short uiClientPort = http_socket.remote_endpoint().port();
        cout<<"Connect:"<<sClientIp<<" "<<uiClientPort<<endl;
    }
    else{
        // perror("connectHandler");
        // cout<<"not connect"<<endl;
        socks4_reply[0]=0;
        socks4_reply[1]=91;
    }

	// printVaraible();
    boost::asio::async_write(socket_, boost::asio::buffer(socks4_reply, 8),
        [this, self](boost::system::error_code ec, std::size_t /*length*/)
        {
          if (!ec)
          {
            Do_Relaying(3);
          }
          else{
            perror("write1");
          }
        });
}

void session::Do_Relaying(int cas){
    auto self(shared_from_this());
    // cout<<"start relaying"<<endl;
    if(cas==3||cas==1){
        http_socket.async_read_some(boost::asio::buffer(http_data_, max_length),
            [this, self](boost::system::error_code ec, std::size_t length)
            {
            if (!ec){
                // cout<<http_data_<<endl<<endl;
                // std::string s = http_data_;
                // cout<<s<<endl;
                boost::asio::async_write(socket_, boost::asio::buffer(http_data_,length),
                    [this, self](boost::system::error_code ec2, std::size_t /*length*/)
                    {
                    if (!ec2)
                    {   
                        memset(http_data_,0,max_length);
                        Do_Relaying(1);
                    }
                    else{
                        perror("write2socket_");
                    }
                    });
            }
            else if (ec ==  boost::asio::error::eof){
                        // cout<<"EOF"<<endl;
            }
            else{
                perror("http read");
            }      
            }
        );
    }
    if(cas==3||cas==2){
        socket_.async_read_some(boost::asio::buffer(client_data_, max_length),
            [this, self](boost::system::error_code ec, std::size_t length)
            {
            if (!ec){
                
                // std::string s = client_data_;
                // cout<<s<<endl;
                boost::asio::async_write(http_socket, boost::asio::buffer(client_data_, length),
                    [this, self](boost::system::error_code ec2, std::size_t /*length*/)
                    {
                    if (!ec2)
                    {
                        memset(client_data_,0,max_length);
                        Do_Relaying(2);
                    }
                    
                    else{
                        perror("write2http");
                    }
                    });
            }
            else if (ec ==  boost::asio::error::eof){
                        // cout<<"EOF"<<endl;
            }
            else{
                // perror("socket_ read");
            }      
            }
        );
    }
 }

void session::printVaraible(){
    cout<<"\n<S_IP>: "<<SRCIP<<endl;
    cout<<"<S_PORT>: "<<SRCPORT<<endl;
    cout<<"<D_IP>: "<<DSTIP<<endl;
    cout<<"<D_PORT>: "<<DSTPORT<<endl;
    cout<<"<Command>: ";
    if(CD==1)
        cout<<"CONNECT"<<endl;
    else
        cout<<"BIND"<<endl;
    cout<<"<Reply>: ";
    if(socks4_reply[1]==90)
        cout<<"Accept"<<endl;
    else
        cout<<"Reject"<<endl;

    // cout<<endl<<"VN: "<<VN<<endl;
    //     cout<<"CD: "<<CD<<endl;
    //     cout<<"DSTIP: "<<DSTIP<<endl;
    //     cout<<"DSTPORT: "<<DSTPORT<<endl;
    //     cout<<"SRCIP: "<<SRCIP<<endl;
    //     cout<<"SRCPORT: "<<SRCPORT<<endl;

    //     if(!DOMAIN_NAME.empty())
    //       cout<<"DOMAIN_NAME: "<<DOMAIN_NAME;
    //     else
    //       cout<<"DOMAIN_NAME is not existed";
    //     cout<<endl;
}