/*/////////////////////////////////////////////////////////////////////
FTPclient.cpp (c) GDI 1999
V1.0.0 (10/4/99)
Phil Anderson. philip@gd-ind.com

Simple FTP client functionality. If you have any problems with it,
please tell me about them (or better still e-mail me the fixed
code). Please feel free to use this code however you wish, although
if you make changes please put your name in the source & comment what
you did.

Nothing awesome going on here at all (all sockets are used in
synchronous blocking mode), but it does the following
things WinInet doesn't seem to:
* Supports loads of different firewalls (I think, I don't
  have access to all types so they haven't all been fully
  tested yet)
* Allows you to execute any command on the FTP server
* Adds 10K to your app install rather than 1Mb #;-)

Functions return TRUE if everything went OK, FALSE if there was an,
error. A message describing the outcome (normally the one returned
from the server) will be in m_retmsg on return from the function.
There are a few error msgs in the app's string table that you'll
need to paste into your app, along with this file & FTPclient.h

If you created your app without checking the "Use Windows Sockets"
checkbox in AppWizard, you'll need to add the following bit of code
to you app's InitInstance()

if(!AfxSocketInit())
{
	AfxMessageBox("Could not initialize Windows Sockets!");
	return FALSE;
}

To use:

1/ Create an object of CFTPclient.

2/ Use LogOnToServer() to connect to the server. Any arguments
not used (e.g. if you're not using a firewall), pass an empty
string or zero for numeric args. You must pass a server port
number, use the FTP default of 21 if you don't know what it is.

3/ Use MoveFile() to upload/download a file, 1st arg is local file
path, 2nd arg is remote file path, 3rd arg is TRUE for a PASV
connection (required by some firewalls), FALSE otherwise, 4th arg
is TRUE to upload, FALSE to download file. MoveFile only works in
synchronous mode (ie the function will not return 'till the transfer
is finished). File transfers are always of type BINARY.

4/ You can use FTPcommand() to execute FTP commands (eg
FTPcommand("CWD /home/mydir") to change directory on the server),
note that this function will return FALSE unless the server response
is a 200 series code. This should work fine for most FTP commands, 
otherwise you can use WriteStr() and ReadStr() to send commands & 
interpret the response yourself. Use LogOffServer() to disconnect
when done.

/////////////////////////////////////////////////////////////////////*/


#include "stdafx.h"
#include "Buco.h"
#include "FTPclient.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFTPclient::CFTPclient()
{
	m_pCtrlsokfile=NULL;
	m_pCtrlTxarch=NULL;
	m_pCtrlRxarch=NULL;
	m_Ctrlsok=NULL;
}


CFTPclient::~CFTPclient()
{
	CloseControlChannel();
}


//////////////////////////////////////////////////////////////////////
// Public Functions
//////////////////////////////////////////////////////////////////////


// function to connect & log on to FTP server
BOOL CFTPclient::LogOnToServer(CString hostname,int hostport,CString username, CString password, CString acct, CString fwhost,CString fwusername, CString fwpassword,int fwport,int logontype) {
	int port,logonpoint=0;
	const int LO=-2, ER=-1;
	CString buf,temp;
	const int NUMLOGIN=9; // currently supports 9 different login sequences
	int logonseq[NUMLOGIN][100] = {
		// this array stores all of the logon sequences for the various firewalls 
		// in blocks of 3 nums. 1st num is command to send, 2nd num is next point in logon sequence array
		// if 200 series response is rec'd from server as the result of the command, 3rd num is next
		// point in logon sequence if 300 series rec'd
		{0,LO,3, 1,LO,6, 2,LO,ER}, // no firewall
		{3,6,3, 4,6,ER, 5,ER,9, 0,LO,12, 1,LO,15, 2,LO,ER}, // SITE hostname
		{3,6,3, 4,6,ER, 6,LO,9, 1,LO,12, 2,LO,ER}, // USER after logon
		{7,3,3, 0,LO,6, 1,LO,9, 2,LO,ER}, //proxy OPEN
		{3,6,3, 4,6,ER, 0,LO,9, 1,LO,12, 2,LO,ER}, // Transparent
		{6,LO,3, 1,LO,6, 2,LO,ER}, // USER with no logon
		{8,6,3, 4,6,ER, 0,LO,9, 1,LO,12, 2,LO,ER}, //USER fireID@remotehost
		{9,ER,3, 1,LO,6, 2,LO,ER}, //USER remoteID@remotehost fireID
		{10,LO,3, 11,LO,6, 2,LO,ER} // USER remoteID@fireID@remotehost
	};

	if(logontype<0||logontype>=NUMLOGIN) return FALSE; // illegal connect code
	// are we connecting directly to the host (logon type 0) or via a firewall? (logon type>0)
	if(!logontype) {
		temp=hostname;
		port=hostport;
	}
	else {
		temp=fwhost;
		port=fwport;
	}
	if(hostport!=21) hostname.Format(hostname+":%d",hostport); // add port to hostname (only if port is not 21)
	if(!OpenControlChannel(temp,port)) return false;
	if(!FTPcommand("")) return FALSE; // get initial connect msg off server
	// go through appropriate logon procedure
	while(1) {
		switch(logonseq[logontype][logonpoint]) {
		case 0:
			temp="USER "+username;
			break;
		case 1:
			temp="PASS "+password;
			break;
		case 2:
			temp="ACCT "+acct;
			break;
		case 3:
			temp="USER "+fwusername;
			break;
		case 4:
			temp="PASS "+fwpassword;
			break;
		case 5:
			temp="SITE "+hostname;
			break;
		case 6:
			temp="USER "+username+"@"+hostname;
			break;
		case 7:
			temp="OPEN "+hostname;
			break;
		case 8:
			temp="USER "+fwusername+"@"+hostname;
			break;
		case 9:
			temp="USER "+username+"@"+hostname+" "+fwusername;
			break;
		case 10:
			temp="USER "+username+"@"+fwusername+"@"+hostname;
			break;
		case 11:
			temp="PASS "+password+"@"+fwpassword;
			break;
		}
		// send command, get response
		if(!WriteStr(temp)) return FALSE;
		if(!ReadStr()) return FALSE;
		// only these responses are valid
		if(m_fc!=2&&m_fc!=3) return FALSE;
		logonpoint=logonseq[logontype][logonpoint+m_fc-1]; //get next command from array
		switch(logonpoint) {
		case ER: // ER means summat has gone wrong
			m_retmsg.LoadString(IDS_FTPMSG1);
			return FALSE;
		case LO: // LO means we're fully logged on
			return TRUE;
		}
	}
}


// function to log off & close connection to FTP server
void CFTPclient::LogOffServer() {
	WriteStr("QUIT");
	CloseControlChannel();
}


// function to execute commands on the FTP server
BOOL CFTPclient::FTPcommand(CString command) {
	if(command!=""&&!WriteStr(command)) return FALSE;
	if((!ReadStr())||(m_fc!=2)) return FALSE;
	return TRUE;
}


// function to upload/download files
BOOL CFTPclient::MoveFile(CString remotefile, CString localfile,BOOL pasv,BOOL get) {
	CString lhost,temp,rhost;
	UINT localsock,serversock,i,j;
	CFile datafile;
	CSocket sockSrvr;
	CAsyncSocket datachannel;
	int num,numread,numsent;
	const int BUFSIZE=4096;
	char cbuf[BUFSIZE];
	DWORD lpArgument=0;
	
	// open local file
	if(!datafile.Open(localfile,(get?CFile::modeWrite|CFile::modeCreate:CFile::modeRead))) {
		m_retmsg.LoadString(IDS_FTPMSG4);
		return FALSE;
	}
	if(!FTPcommand("TYPE I")) return FALSE; // request BINARY mode
	if(pasv) { // set up a PASSIVE type file transfer
		if(!FTPcommand("PASV")) return FALSE;
		// extract connect port number and IP from string returned by server
		if((i=m_retmsg.Find("("))==-1||(j=m_retmsg.Find(")"))==-1) return FALSE;
		temp=m_retmsg.Mid(i+1,(j-i)-1);
		i=temp.ReverseFind(',');
		serversock=atol(temp.Right(temp.GetLength()-(i+1))); //get ls byte of server socket
		temp=temp.Left(i);
		i=temp.ReverseFind(',');
		serversock+=256*atol(temp.Right(temp.GetLength()-(i+1))); // add ms byte to server socket
		rhost=temp.Left(i);
		while(1) { // convert commas to dots in IP
			if((i=rhost.Find(","))==-1) break;
			rhost.SetAt(i,'.');
		}
	}
	else { // set up a ACTIVE type file transfer
		m_retmsg.LoadString(IDS_FTPMSG6);
		// get the local IP address off the control channel socket
		if(!m_Ctrlsok->GetSockName(lhost,localsock)) return FALSE;;
		while(1) { // convert returned '.' in ip address to ','
			if((i=lhost.Find("."))==-1) break;
			lhost.SetAt(i,',');
		}
		// create listen socket (let MFC choose the port) & start the socket listening
		if((!sockSrvr.Create(0,SOCK_STREAM,NULL))||(!sockSrvr.Listen())) return FALSE;
		if(!sockSrvr.GetSockName(temp,localsock)) return FALSE;// get the port that MFC chose
		// convert the port number to 2 bytes + add to the local IP
		lhost.Format(lhost+",%d,%d",localsock/256,localsock%256);
		if(!FTPcommand("PORT "+lhost)) return FALSE;// send PORT cmd to server
	}
	// send RETR/STOR command to server
	if(!WriteStr((get?"RETR ":"STOR ")+remotefile)) return FALSE;
	if(pasv) {// if PASV create the socket & initiate outbound data channel connection
		if(!datachannel.Create()) {
			m_retmsg.LoadString(IDS_FTPMSG6);
			return FALSE;
		}
		datachannel.Connect(rhost,serversock); // attempt to connect asynchronously (server will tell us if/when we're connected)
	}
	if(!ReadStr()||m_fc!=1) return FALSE; // get response to RETR/STOR command
	if(!pasv&&!sockSrvr.Accept(datachannel)) return FALSE; // if !PASV accept inbound data connection from server
	// we're connected & ready to do the data transfer, so set blocking mode on data channel socket
	if((!datachannel.AsyncSelect(0))||(!datachannel.IOCtl(FIONBIO,&lpArgument))) {
		m_retmsg.LoadString(IDS_FTPMSG6);
		return FALSE;
	}
	while(1) { // move data from/to server & read/write local file
		TRY {
			if(get) {
				if(!(num=datachannel.Receive(cbuf,BUFSIZE,0))||num==SOCKET_ERROR) break; // (EOF||network error)
				else datafile.Write(cbuf,num);
			}
			else {
				if(!(numread=datafile.Read(cbuf,BUFSIZE))) break; //EOF
				if((numsent=datachannel.Send(cbuf,numread,0))==SOCKET_ERROR) break;
				// if we sent fewer bytes than we read from file, rewind file pointer
				if(numread!=numsent) datafile.Seek(numsent-numread,CFile::current);
			}
		}
		CATCH (CException,e) {
			m_retmsg.LoadString(IDS_FTPMSG5);
			return FALSE;
		}
		END_CATCH
	}
	datachannel.Close();
	datafile.Close();
	if(!FTPcommand("")) return FALSE; // check transfer outcome msg from server
	return TRUE; // oh goody it worked.
}


// function to send a command string on the server control channel
BOOL CFTPclient::WriteStr(CString outputstring) {
	m_retmsg.LoadString(IDS_FTPMSG6); // pre-load "network error" msg (in case there is one) #-)
	TRY {
		m_pCtrlTxarch->WriteString(outputstring+"\r\n");
		m_pCtrlTxarch->Flush();
	}
	CATCH(CException,e) {
		return FALSE;
	}
	END_CATCH
	return TRUE;
}


// this function gets the server response line
BOOL CFTPclient::ReadStr() {
	int retcode;

	if(!ReadStr2()) return FALSE;
	if(m_retmsg.GetLength()<4||m_retmsg.GetAt(3)!='-') return TRUE;
	retcode=atol(m_retmsg);
	while(1) { //handle multi-line server responses
		if(m_retmsg.GetLength()>3&&(m_retmsg.GetAt(3)==' '&&atol(m_retmsg)==retcode)) return TRUE;
		if(!ReadStr2()) return FALSE;
	}
}



//////////////////////////////////////////////////////////////////////
// Private functions
//////////////////////////////////////////////////////////////////////


// read a single response line from the server control channel
BOOL CFTPclient::ReadStr2() {
	TRY {
		if(!m_pCtrlRxarch->ReadString(m_retmsg)) {
			m_retmsg.LoadString(IDS_FTPMSG6);
			return FALSE;
		}
	}
	CATCH(CException,e) {
		m_retmsg.LoadString(IDS_FTPMSG6);
		return FALSE;
	}
	END_CATCH
	if(m_retmsg.GetLength()>0) m_fc=m_retmsg.GetAt(0)-48; // get 1st digit of the return code (indicates primary result)
	return TRUE;
}


// open the control channel to the FTP server
BOOL CFTPclient::OpenControlChannel(CString serverhost,int serverport) {
	m_retmsg.LoadString(IDS_FTPMSG2);
	if(!(m_Ctrlsok=new CSocket)) return FALSE;
	if(!(m_Ctrlsok->Create())) return FALSE;
	m_retmsg.LoadString(IDS_FTPMSG3);
	if(!(m_Ctrlsok->Connect(serverhost,serverport))) return FALSE;
	m_retmsg.LoadString(IDS_FTPMSG2);
	if(!(m_pCtrlsokfile=new CSocketFile(m_Ctrlsok))) return FALSE;
	if(!(m_pCtrlRxarch=new CArchive(m_pCtrlsokfile,CArchive::load))) return FALSE;
	if(!(m_pCtrlTxarch=new CArchive(m_pCtrlsokfile,CArchive::store))) return FALSE;
	return TRUE;
}


// close the control channel to the FTP server
void CFTPclient::CloseControlChannel() {
	if(m_pCtrlTxarch) delete m_pCtrlTxarch;
	m_pCtrlTxarch=NULL;
	if(m_pCtrlRxarch) delete m_pCtrlRxarch;
	m_pCtrlRxarch=NULL;
	if(m_pCtrlsokfile) delete m_pCtrlsokfile;
	m_pCtrlsokfile=NULL;
	if(m_Ctrlsok) delete m_Ctrlsok;
	m_Ctrlsok=NULL;
	return;
}





