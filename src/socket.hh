class Socket {
 public:
    virtual void listen();
    virtual void write();
    virtual void read();
    virtual void connect();

    virtual ~Socket();
};