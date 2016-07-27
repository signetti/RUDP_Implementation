class UserInterfaceC
{
public:
	static UserInterfaceC	*CreateInstance();
	static UserInterfaceC	*GetInstance() {return sInstance;};
	~UserInterfaceC(){};
	
	void		render();

    void        rightMousePressed();

private:
	static void drawString(int32_t x, int32_t y, char* format, ...);
	static void drawStringNoRaster(char* format, ...);

	static UserInterfaceC*	sInstance;
	UserInterfaceC(){};
};