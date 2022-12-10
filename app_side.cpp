int testappfunc()
{
	#define myline std::cout << __FUNCTION__ << ", " << __LINE__ << "." << std::endl;;
	int retval=0;
	int appcnt=0;
	int fd = socket(AF_LOCAL, SOCK_SEQPACKET, 0);
	myline;
	struct sockaddr_un myunc;
	strcpy(myunc.sun_path, "/tmp/single.socket");
	myline;
	
	retval = connect(fd, (struct sockaddr*)&myunc, sizeof(myunc));
	if(retval == -1)
	{
		perr("connect error");
		myline;
		exit(1);
	}
	
	myline;
	while (appcnt < 20)
	{
		char buf[64] = {0};
		char buf2[64] = {0};
		sprintf(buf, "appcnt=%d.", appcnt++);
		myline;
		std::cout << " buf=" << buf << std::endl;
		retval = send(fd, buf, sizeof(buf), 0);
		if(retval == -1)
		{
			perror("send error");
			std::cout << "retval=" << retval << std::endl;
			myline;
			exit(1);
		}
		myline;
		retval = recv(fd, buf2, sizeof(buf2), 0);
		myline;
		std::cout << "retval=" << retval << ", fd=" << fd << ", buf2=" << buf2 << std::endl;
		sleep(1);
	}
	myline;
}
