// SPDX: AGPL-3.0-only
/* XEPL Operating Environment - Copyright (c) 2024 Keith Edwin Robbins
	Project Name: XEPL Operating Environment
	File Name:    xepl_sockets_kit.hpp
	Author:       Keith Edwin Robbins
	Release date: May 10, 2024
	Website:      https://xepl.com

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU Affero General Public License as published by
	the Free Software Foundation version 3 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Affero General Public License for more details.

	You should have received a copy of the GNU Affero General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>.

	For more information about the AGPL, please visit:
 	https://www.gnu.org/licenses/agpl-3.0.html
*/


namespace KITS::SOCKETS
{
	void Keyword_SocketMan   ( XEPL::Cortex* _cortex );
	void Keyword_Tcpserver   ( XEPL::Cortex* _cortex );
	void Register_Socket_Kit ( XEPL::Cortex* _cortex );
}


namespace KITS::OS
{
	class osFd;
	class osFdPair;
	class osFdSelect;
	class osFdSet;
	class osSocket;
	class osSocketAddress;
	class osSocketControl;
	class osTcpServer;
	class osTcpSocket;	
	
	typedef bool ( osFd::*osFdHandler ) ( void );
}


namespace KITS::SOCKETS
{
	class Socket;
	class SocketMan;
	class SocketControl;
	class SocketTcp;
	class TcpServer;

	class Socket : public XEPL::Neuron
	{
	protected:
		virtual ~Socket ( void ) override;
	public:
		OS::osSocket*  os_socket;
		XEPL::Gene*    config_gene;
		explicit Socket ( XEPL::Neuron*, XEPL::Gene* );
		virtual void Nucleus_Dropped( void ) override;
	};

	class SocketTcp : public Socket
	{
		friend class KITS::OS::osTcpSocket;
		OS::osTcpSocket*    os_tcp_socket;
		bool                aborted;
		virtual void Nucleus_Dropped( void ) override;
		void Closed_Socket_Write      ( void );
		void Closed_Socketman_Read    ( void );
		void Closed_Socketman_Write   ( void );
		void Method_Closed            ( XEPL::Gene*, XEPL::Gene* );
		void Method_SendContent       ( XEPL::Gene*, XEPL::Gene* );
		void Method_Start             ( XEPL::Gene*, XEPL::Gene* );
	protected:
		XEPL::Axon*       closed_axon;
		void Set_Tcp_Socket ( OS::osTcpSocket* );
		virtual ~SocketTcp  ( void ) override;
	public:
		explicit SocketTcp ( XEPL::Neuron*,    XEPL::Gene* );
		explicit SocketTcp ( OS::osTcpSocket*, XEPL::Neuron*, XEPL::Gene* );
	};

	class SocketControl : public Socket
	{
	public:
		OS::osSocketControl*  control_socket;
		explicit SocketControl (SocketMan* );
		void Send ( void );
	};

	class SocketMan : public XEPL::Lobe
	{
		friend class SocketControl;
		virtual void Lobe_Born      ( void ) override;
		virtual void Lobe_Dying     ( void ) override;
		virtual void Lobe_Rest_Now  ( void ) override;
		virtual void Lobe_Wake_Up   ( void ) override;
		void Do_Read      ( XEPL::Atom*, XEPL::Atom* );
		void Do_Write     ( XEPL::Atom*, XEPL::Atom* );
		void Do_Cancel    ( XEPL::Atom*, XEPL::Atom* );
		SocketControl*   socket_control;
		OS::osFdSet*     fdSet;
	public:
		XEPL::Axon*      read_axon;
		XEPL::Axon*      write_axon;
		XEPL::Axon*      cancel_axon;
		explicit SocketMan ( XEPL::Neuron*, XEPL::Gene* );
		void Unhook ( OS::osFd* );
	protected:
		virtual ~SocketMan ( void ) override;
	};


	class TcpServer : public SocketTcp
	{
		friend class OS::osTcpServer;
		XEPL::Axon*        connecting_axon;
		XEPL::String*      node_string;
		OS::osTcpServer*   server_socket;
	protected:
		virtual ~TcpServer ( void ) override;
	public:
		explicit TcpServer ( XEPL::Neuron*, XEPL::Gene* );
	private:
		void Set_Server_Socket    ( OS::osTcpServer* );
		void Server_Create        ( void );
		void Listen               ( void );
		bool SocketMan_Connecting ( int,  OS::osSocketAddress* );
		void Client_Connected     ( XEPL::Atom*, XEPL::Atom* );
	};
	
}


struct sockaddr;
struct sockaddr_in;

#ifdef _WIN32
#include <winsock2.h>
#else
typedef int SOCKET;
#endif

namespace KITS::OS
{
	class osFdHandlerMap : public XEPL::UMapT< SOCKET, std::pair<OS::osFd*, OS::osFdHandler> > {};

	class osFd : public XEPL::Atom
	{
		friend class OS::osFdPair;
		friend class SOCKETS::SocketMan;
		bool Fd_Does_Nothing( void );
		virtual SOCKET osFd_Descriptor      ( void ) = 0;
		virtual osFdHandler osFd_OnFdRead   ( void ) { return ( osFdHandler )&osFd::Fd_Does_Nothing; }
		virtual osFdHandler osFd_OnFdWrite  ( void ) { return ( osFdHandler )&osFd::Fd_Does_Nothing; }
	protected:
		XEPL::String*         socket_name;
		XEPL::Gene*           receiving_gene;
		XEPL::Wire*           backpressure_wire;
		XEPL::Mutex*          fd_mutex;
		virtual ~osFd ( void ) override;
		explicit osFd ( void );
		void Receive_From_Fd  ( XEPL::Text*, long );
	public:
		SOCKETS::SocketMan*    socket_manager;
		XEPL::Axon*           eReceived;
		bool Set_Manager       ( XEPL::Neuron* );
		void Set_Data_Axon     ( XEPL::Axon*   );
		void Add_Backpressure  ( XEPL::Wire*   );
		bool Has_Backpressure  ( void );
	};

	class osFdSelect : public XEPL::NoCopy
	{
		friend class osFdPair;
		::fd_set*   os_fd_set;
	public:
		~osFdSelect();
		osFdSelect ( void );
	private:

		void Set_Fd        ( int );
		bool Is_Fd_Ready   ( int );
		void Clear_Fd      ( int );
		int  Deliver_Fds   ( osFdPair*, int );
	};

	class osFdPair : public XEPL::NoCopy
	{
		friend class osFdSelect;
		friend class osFdSet;
		friend class SOCKETS::SocketMan;
		using SetInts = std::set<int, std::less<int>, XEPL::IntAllocator>;
	protected:
		SetInts*              all_fds;
		OS::osFdHandlerMap*   handler_map;
		OS::osFdSelect*       watch_fds;
		OS::osFdSelect*       active_fds;
		int                   num_fds;
		int                   max_fd;
	public:
		osFdPair ( void );
		~osFdPair ( void );
	protected:
		void Remove_Fd       ( int );
		void Ignore_Fd       ( osFd* file_descriptor );
		void Set_Fd_Handler  ( osFd* file_descriptor, osFdHandler to_do );
	private:
		int  Max_Fd          ( int current_max_fd );
		int  Deliver_Fd      ( int number_of_active_descriptors );
		void Clear_Bad_Fds   ( void );
		::fd_set* Active_Fds ( void );
	};

	class osFdSet : public XEPL::NoCopy
	{
		friend class SOCKETS::SocketMan;
		osFdPair*     read_pair;
		osFdPair*     write_pair;
	public:
		~osFdSet ( void );
		osFdSet  ( void );
	private:
		void Ignore            ( osFd*, osFdPair* );
		void Wait_On_Selected  ( void );
	};
}


namespace KITS::OS
{
	class osSocket : public osFd
	{
	protected:
		SOCKET                 socket_fd;
		SOCKETS::Socket*       socket_neuron;
	public:
		osSocketAddress*      socket_address;
	protected:
		friend class SOCKETS::Socket;	
		virtual ~osSocket ( void ) override;
		explicit osSocket ( SOCKETS::Socket* );
		explicit osSocket ( SOCKETS::Socket*, int, osSocketAddress* );
		virtual SOCKET  osFd_Descriptor( void ) override;
		void Bind_Socket  ( bool );
		void Build_Socket ( int  );
		void Close_Socket ( void );
	};

	class osSocketAddress : public XEPL::NoCopy
	{
		::sockaddr_in*           sockAddr;
		XEPL::String*            ip_string;
	public:
		~osSocketAddress         ( void );
		osSocketAddress          ( void );
		explicit osSocketAddress ( XEPL::Gene* );
		long Length              ( void );
		long Port                ( void );
		XEPL::Cord* IpString     ( void );
		::sockaddr* Get          ( void );
		static void Hostname     ( XEPL::String* );
	};

	class osSocketControl : public osSocket
	{
		SOCKETS::SocketControl*   socket_control;
	public:
		explicit osSocketControl ( SOCKETS::SocketControl* );
	private:
		friend class SOCKETS::SocketControl;
		virtual osFdHandler osFd_OnFdRead ( void ) override;
		bool    Fd_Receive       ( void );
		void    Send             ( void );
	};

	class osTcpSocket : public osSocket
	{
		virtual osFdHandler osFd_OnFdRead  ( void ) override {
			return ( osFdHandler )&osTcpSocket::Read_Available;
		}
		virtual osFdHandler osFd_OnFdWrite ( void ) override {
			return ( osFdHandler )&osTcpSocket::Send_Backpressure;
		}
	public:
		SOCKETS::SocketTcp*             socket_tcp;
		explicit osTcpSocket         ( SOCKETS::SocketTcp* );
		explicit osTcpSocket         ( SOCKETS::SocketTcp*, int, osSocketAddress* );
		void Create_Tcp_Socket       ( void );
		void Send_Data               ( XEPL::Wire* );
		bool Read_Available          ( void );
		bool Send_Backpressure       ( void );
	};

	class osTcpServer : public osTcpSocket
	{
		SOCKETS::TcpServer*       tcp_server;
		virtual osFdHandler osFd_OnFdRead ( void ) override {
			return ( osFdHandler )&osTcpServer::Fd_Is_Connecting;
		}
	protected:
		virtual ~osTcpServer() override;
	public:
		explicit osTcpServer         ( SOCKETS::TcpServer* );
		void Listen_For_Connections  ( long );
		bool Fd_Is_Connecting        ( void );
	};
}



KITS::SOCKETS::Socket::~Socket()
{
	if ( config_gene )
		config_gene->Release();

	if ( os_socket )
	{
		os_socket->Close_Socket();
		os_socket->Release();
	}
}

KITS::SOCKETS::Socket::Socket ( XEPL::Neuron*  _neuron, XEPL::Gene* _gene )
	: Neuron      ( _neuron, _gene )
	, os_socket   ( nullptr )
	, config_gene ( _gene )
{
	if ( _gene )
		_gene->Attach();
}

void KITS::SOCKETS::Socket::Nucleus_Dropped()
{
	os_socket->Close_Socket();
	Neuron::Nucleus_Dropped();
}



KITS::SOCKETS::SocketControl::SocketControl ( KITS::SOCKETS::SocketMan* _socket_man )
	: Socket         ( _socket_man, _socket_man->shadows->Make_One("config") )
	, control_socket ( new KITS::OS::osSocketControl ( this ) )
{
	Socket::os_socket = control_socket;

	XEPL::Spike* spike = new XEPL::Spike ( control_socket );

	_socket_man->Do_Read ( spike, nullptr );

	spike->Release();
}

void KITS::SOCKETS::SocketControl::Send()
{
	control_socket->Send();
}




KITS::SOCKETS::SocketMan::~SocketMan()
{
	delete fdSet;
}

KITS::SOCKETS::SocketMan::SocketMan ( XEPL::Neuron*  _parent, XEPL::Gene* _config )
	: XEPL::Lobe      ( _parent, _config )
	, socket_control  ( nullptr )
	, fdSet           ( new KITS::OS::osFdSet() )
	, read_axon       ( new XEPL::Axon ( this, "Read"  ) )
	, write_axon      ( new XEPL::Axon ( this, "Write" ) )
	, cancel_axon     ( new XEPL::Axon ( this, "Cancel") )
{}

void KITS::SOCKETS::SocketMan::Lobe_Wake_Up()
{
	socket_control->Send();
}

void KITS::SOCKETS::SocketMan::Lobe_Born()
{
	shadows->Make_One("config")->Trait_Set ( "node", "127.0.0.1:0" );

	Synapse_Axon ( read_axon,   ( XEPL::Receiver )&SocketMan::Do_Read,   nullptr );
	Synapse_Axon ( write_axon,  ( XEPL::Receiver )&SocketMan::Do_Write,  nullptr );
	Synapse_Axon ( cancel_axon, ( XEPL::Receiver )&SocketMan::Do_Cancel, nullptr );

	socket_control  = new KITS::SOCKETS::SocketControl ( this );

	XEPL::Lobe::Lobe_Born();
}

void KITS::SOCKETS::SocketMan::Lobe_Dying()
{
	Unhook ( socket_control->os_socket );
	XEPL::Lobe::Lobe_Dying();
}

void KITS::SOCKETS::SocketMan::Lobe_Rest_Now()
{
	++XEPL::tlsLobe->counters.count_wakes;
	fdSet->Wait_On_Selected();
}

void KITS::SOCKETS::SocketMan::Do_Read ( XEPL::Atom* _impulse, XEPL::Atom* )
{
	XEPL::Spike* impulse = static_cast<XEPL::Spike*> ( _impulse );

	OS::osFd* fd = static_cast<KITS::OS::osFd*> ( impulse->stimulus );

	fdSet->read_pair->Set_Fd_Handler ( fd, fd->osFd_OnFdRead() );
}

void KITS::SOCKETS::SocketMan::Do_Write ( XEPL::Atom* _impulse, XEPL::Atom* )
{
	XEPL::Spike* impulse = static_cast<XEPL::Spike*> ( _impulse );

	OS::osFd* fd = static_cast<KITS::OS::osFd*> ( impulse->stimulus );

	fdSet->write_pair->Set_Fd_Handler ( fd, fd->osFd_OnFdWrite() );
}

void KITS::SOCKETS::SocketMan::Do_Cancel ( XEPL::Atom* _impulse, XEPL::Atom* )
{
	XEPL::Spike* impulse = static_cast<XEPL::Spike*> ( _impulse );

	OS::osFd* fd = static_cast<KITS::OS::osFd*>( impulse->stimulus );

	Unhook ( fd );
}

void KITS::SOCKETS::SocketMan::Unhook ( OS::osFd* _fd )
{
	fdSet->Ignore ( _fd, fdSet->read_pair  );
	fdSet->Ignore ( _fd, fdSet->write_pair );
}



KITS::SOCKETS::SocketTcp::~SocketTcp()
{
	delete alias;
	alias = nullptr;
}

KITS::SOCKETS::SocketTcp::SocketTcp ( XEPL::Neuron* _parent, XEPL::Gene* _config )
	: Socket         ( _parent, _config )
	, os_tcp_socket  ( nullptr )
	, aborted        ( false )
	, closed_axon    ( new XEPL::Axon ( this, "closed" ) )
{
	Register_Method ( "SendContent", ( XEPL::Function )&SocketTcp::Method_SendContent, nullptr );
	Register_Method ( "Start",       ( XEPL::Function )&SocketTcp::Method_Start,       nullptr );
	Register_Method ( "Closed",      ( XEPL::Function )&SocketTcp::Method_Closed,      nullptr );
}

KITS::SOCKETS::SocketTcp::SocketTcp ( KITS::OS::osTcpSocket* _client, XEPL::Neuron*  _parent, XEPL::Gene* _config )
	: Socket         ( _parent, _config )
	, os_tcp_socket  ( _client )
	, aborted        ( false )
	, closed_axon    ( new XEPL::Axon ( this, "closed" ) )
{
	os_socket = os_tcp_socket;
	os_tcp_socket->socket_tcp = this;

	Register_Method ( "SendContent", ( XEPL::Function )&SocketTcp::Method_SendContent, nullptr );
	Register_Method ( "Start",       ( XEPL::Function )&SocketTcp::Method_Start,       nullptr );
	Register_Method ( "Closed",      ( XEPL::Function )&SocketTcp::Method_Closed,      nullptr );
}

void KITS::SOCKETS::SocketTcp::Set_Tcp_Socket ( KITS::OS::osTcpSocket* _tcp )
{
	os_socket     = _tcp;
	os_tcp_socket = _tcp;
}

void KITS::SOCKETS::SocketTcp::Nucleus_Dropped()
{
	if ( Test_Flags(XEPL::dropped_flag) )
		return;

	KITS::SOCKETS::SocketMan* socket_manager = os_socket->socket_manager;

	if ( socket_manager )
		socket_manager->cancel_axon->Trigger_Wait ( os_tcp_socket );

	Socket::Nucleus_Dropped();
}

void KITS::SOCKETS::SocketTcp::Closed_Socket_Write()
{
	aborted = true;
	closed_axon->Trigger ( nullptr );
}

void KITS::SOCKETS::SocketTcp::Closed_Socketman_Read()
{
	KITS::SOCKETS::SocketMan* socket_manager = os_socket->socket_manager;
	socket_manager->Unhook( os_socket );
	closed_axon->Trigger ( nullptr );
}

void KITS::SOCKETS::SocketTcp::Closed_Socketman_Write()
{
	KITS::SOCKETS::SocketMan* socket_manager = os_socket->socket_manager;
	socket_manager->Unhook( os_socket );
	closed_axon->Trigger ( nullptr );
}

void KITS::SOCKETS::SocketTcp::Method_Closed ( XEPL::Gene*, XEPL::Gene* )
{
	Nucleus_Dropped();
}

void KITS::SOCKETS::SocketTcp::Method_SendContent ( XEPL::Gene* _call_gene, XEPL::Gene* )
{
	XEPL::Wire* sending_wire = XEPL::tlsLobe->index_link->content_wire;
	if ( sending_wire )
	{
		while ( sending_wire->Avail() && !aborted )
		{
			XEPL::MutexScope lock_string ( sending_wire->wire_mutex );

			if ( os_tcp_socket->Has_Backpressure() )
			{
				os_tcp_socket->Add_Backpressure ( sending_wire );
				break;
			}
			os_tcp_socket->Send_Data ( sending_wire );
		}
	}

	if ( _call_gene->inner_genes )
		Process_Inner_Genes ( _call_gene );
}

void KITS::SOCKETS::SocketTcp::Method_Start ( XEPL::Gene*, XEPL::Gene* )
{
	os_socket->socket_manager->read_axon->Trigger_Wait ( os_tcp_socket );
}



namespace KITS::SOCKETS
{
	/// Ephemeral socket of Tcp Connection
	class ConnectedClient : public XEPL::Atom
	{
		friend class TcpServer;

		int                   descriptor;
		OS::osSocketAddress*  address;

		ConnectedClient ( int _descriptor, OS::osSocketAddress* _address )
			: Atom()
			, descriptor ( _descriptor )
			, address    ( _address )
		{}
	};
}

KITS::SOCKETS::TcpServer::~TcpServer()
{
	delete node_string;
}

KITS::SOCKETS::TcpServer::TcpServer ( XEPL::Neuron*  _parent, XEPL::Gene* _config )
	: SocketTcp ( _parent, _config )
	, connecting_axon ( new XEPL::Axon ( this, "connecting" ) )
	, node_string     ( nullptr )
	, server_socket   ( nullptr )
{
	Synapse_Axon ( connecting_axon, ( XEPL::Receiver )&TcpServer::Client_Connected, config_gene );
	Set_Server_Socket ( new KITS::OS::osTcpServer ( this ) );
	Server_Create();
	Listen();
}

void KITS::SOCKETS::TcpServer::Set_Server_Socket ( KITS::OS::osTcpServer* _tcp )
{
	server_socket = _tcp;
	server_socket->Set_Manager ( this );
	Set_Tcp_Socket ( _tcp );
}

void KITS::SOCKETS::TcpServer::Server_Create ( void )
{
	node_string = new XEPL::String ( config_gene->Trait_Tap ( "node", "127.0.0.1" ) );
	os_socket->socket_address = new OS::osSocketAddress ( config_gene );
	server_socket->Create_Tcp_Socket();
}

void KITS::SOCKETS::TcpServer::Listen ( void )
{
	if ( server_socket->socket_manager )
	{
		server_socket->Listen_For_Connections ( 10 );

		XEPL::Spike* spike = new XEPL::Spike ( server_socket );

		server_socket->socket_manager->read_axon->Trigger ( spike );

		spike->Release();
	}
	else
		XEPL::ErrorReport error_report( "No Socketman for ", node_string );
}

void KITS::SOCKETS::TcpServer::Client_Connected ( XEPL::Atom* _impulse, XEPL::Atom* _atom )
{
	ConnectedClient* connectedTo = static_cast<ConnectedClient*> ( _impulse );

	KITS::OS::osTcpSocket* client = new KITS::OS::osTcpSocket ( this, connectedTo->descriptor, connectedTo->address );

	XEPL::DuplicateTraits clone ( static_cast<XEPL::Gene*> ( _atom ) );

	XEPL::String node_name( clone.gene->Trait_Raw ( "name" ) );
	node_name.append ( std::to_string ( client->socket_address->Port() ) );

	clone.gene->Trait_Set( "name", &node_name );

	SocketTcp* newSocket = new SocketTcp ( client, this, clone.gene );

	XEPL::ShortTerms push_terms( "from", client->socket_address->IpString() );

	client->Set_Manager   ( this );
	client->Set_Data_Axon ( new XEPL::Axon ( newSocket, "eReceived" ) );

	newSocket->alias = new XEPL::Cord ( cell_name );
	newSocket->Process_Inner_Genes ( clone.gene );
}

bool KITS::SOCKETS::TcpServer::SocketMan_Connecting ( int _socket, OS::osSocketAddress* _address )
{
	ConnectedClient* client  = new ConnectedClient ( _socket, _address );
	connecting_axon->Trigger ( client );
	client->Release();
	return true;
}



/// Socket OS Porting Layer


#if _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#else
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#endif


KITS::OS::osFdSelect::~osFdSelect ()
{
	delete os_fd_set;
}

KITS::OS::osFdSelect::osFdSelect ( void )
	: os_fd_set ( new ::fd_set() )
{
	FD_ZERO ( os_fd_set );
}

void KITS::OS::osFdSelect::Set_Fd ( int _fd )
{
	FD_SET ( _fd, os_fd_set );
}

void KITS::OS::osFdSelect::Clear_Fd ( int _fd )
{
	FD_CLR ( _fd, os_fd_set );
}

bool KITS::OS::osFdSelect::Is_Fd_Ready ( int _fd )
{
	return FD_ISSET ( _fd, os_fd_set ) != 0;
}

int KITS::OS::osFdSelect::Deliver_Fds ( osFdPair* _pair, int _num_fds )
{
	int delivered = 0;
	int        fd = _pair->max_fd;

	while ( _num_fds && ( fd >= 0 ) )
	{
		if ( _pair->active_fds->Is_Fd_Ready ( fd ) )
		{
			_num_fds--;

			auto it  = _pair->handler_map->find ( fd );
			if ( it != _pair->handler_map->end() )
			{
				if ( ! ( it->second.first->*it->second.second )() )
					_pair->Ignore_Fd ( it->second.first );

				++delivered;
			}
			else
				; // hmmm?
		}
		fd--;
	}
	return delivered;
}


KITS::OS::osFdPair::~osFdPair()
{
	delete watch_fds;
	delete active_fds;
	delete all_fds;
	delete handler_map;
}

KITS::OS::osFdPair::osFdPair()
	: all_fds      ( new SetInts() )
	, handler_map  ( new osFdHandlerMap() )
	, watch_fds    ( new osFdSelect() )
	, active_fds   ( new osFdSelect() )
	, num_fds      ( 0 )
	, max_fd       ( 0 )
{}

int KITS::OS::osFdPair::Deliver_Fd ( int _num_fds )
{
	return active_fds->Deliver_Fds ( this, _num_fds );
}

void KITS::OS::osFdPair::Set_Fd_Handler ( osFd* _os_fd, osFdHandler _handler )
{
	int fd = _os_fd->osFd_Descriptor();

	if ( fd < 0 )
		return;

	const auto& [it, noob] = handler_map->insert_or_assign(fd, std::pair<osFd*,osFdHandler>(_os_fd, _handler));
	if ( noob )
	{
		watch_fds->Set_Fd   ( fd );
		all_fds->insert ( fd );

		++num_fds;
		max_fd = * ( all_fds->rbegin() );
	}
}

fd_set* KITS::OS::osFdPair::Active_Fds( void )
{
	if ( !num_fds )
		return nullptr;

	FD_ZERO(active_fds->os_fd_set);

	for (int fd : *all_fds )
	{
		FD_SET(fd, active_fds->os_fd_set);
	}

	return active_fds->os_fd_set;
}

void KITS::OS::osFdPair::Clear_Bad_Fds(void)
{
    if (!max_fd)
        return;

    SetInts fds(all_fds->begin(), all_fds->end());

    fd_set check_fds;
    struct timeval zero_time = {0, 0};

    for (int fd : fds)
    {
        FD_ZERO(&check_fds);
        FD_SET(fd, &check_fds);

        if (select(fd + 1, nullptr, nullptr, &check_fds, &zero_time) == -1)
            Remove_Fd(fd);
    }
}

void KITS::OS::osFdPair::Remove_Fd ( int _fd )
{
	auto it  = handler_map->find ( _fd );
	if ( it == handler_map->end() )
		return;

	handler_map->erase    ( it );
	all_fds->erase        ( _fd );
	watch_fds->Clear_Fd   ( _fd );
	active_fds->Clear_Fd  ( _fd );

	if ( --num_fds )
		max_fd = * ( all_fds->rbegin() );
	else
		max_fd = 0;
}

void KITS::OS::osFdPair::Ignore_Fd ( osFd* _socket )
{
	int fd = _socket->osFd_Descriptor();
	Remove_Fd(fd);
}

int KITS::OS::osFdPair::Max_Fd ( int _fd )
{
	if ( _fd > max_fd )
		return _fd;

	return max_fd;
}


KITS::OS::osFd::~osFd()
{
	delete fd_mutex;
	delete socket_name;

	if ( receiving_gene )
		receiving_gene->Release();

	if ( backpressure_wire )
		backpressure_wire->Release();
}

KITS::OS::osFd::osFd ( void )
	: XEPL::Atom()
	, socket_name       ( nullptr )
	, receiving_gene    ( nullptr )
	, backpressure_wire ( nullptr )
	, fd_mutex          ( nullptr )
	, socket_manager    ( nullptr )
	, eReceived         ( nullptr )
{}

bool KITS::OS::osFd::Set_Manager ( XEPL::Neuron* _neuron )
{
	XEPL::Neuron* neuron = nullptr;

	if ( _neuron->Find_Neuron ( "SocketMan", &neuron ) )
		socket_manager = static_cast<KITS::SOCKETS::SocketMan*> ( neuron );
	else
		XEPL::ErrorReport ("Can't locate SocketMan");

	return socket_manager;
}

bool KITS::OS::osFd::Fd_Does_Nothing( void )
{
	return false;
}

void KITS::OS::osFd::Set_Data_Axon ( XEPL::Axon* _axon )
{
	eReceived = _axon;
}

void KITS::OS::osFd::Receive_From_Fd ( XEPL::Text* _chars, long _length )
{
	if ( !receiving_gene )
	{
		if ( !socket_name )
		{
			socket_name = new XEPL::String ( "fd_" );
			socket_name->append ( std::to_string ( osFd_Descriptor() ) );
		}
		receiving_gene = new XEPL::Gene ( nullptr, socket_name );
	}
	receiving_gene->Append_Content ( _chars, _length );
}

void KITS::OS::osFd::Add_Backpressure ( XEPL::Wire* _locked_wire )
{
	if ( !fd_mutex )
		fd_mutex = new XEPL::Mutex;

	XEPL::MutexScope lock_backpressure ( fd_mutex );

	if ( !backpressure_wire )
		backpressure_wire = new XEPL::Wire();

	backpressure_wire->wire_string->append ( *_locked_wire->wire_string );
	
	_locked_wire->wire_string->clear ();
}

bool KITS::OS::osFd::Has_Backpressure()
{
	if ( !fd_mutex )
		return false;

	XEPL::MutexScope lock_backpressure ( fd_mutex );

	if ( !backpressure_wire )
		return false;

	return backpressure_wire->Avail() > 0;
}


KITS::OS::osFdSet::osFdSet()
	: read_pair    ( new osFdPair() )
	, write_pair   ( new osFdPair() )
{}

KITS::OS::osFdSet::~osFdSet()
{
	delete write_pair;
	delete read_pair;
}

void KITS::OS::osFdSet::Ignore ( osFd* _fd, osFdPair* _pair )
{
	if ( !_pair->num_fds )
		return;
	
	_pair->Ignore_Fd ( _fd );
}

void KITS::OS::osFdSet::Wait_On_Selected()
{
	int max_fd = read_pair->Max_Fd ( write_pair->Max_Fd ( 0 ) );
	int active_count = ::select ( max_fd+1, read_pair->Active_Fds(), write_pair->Active_Fds(), nullptr, nullptr);

	if ( active_count <= 0 )
	{
		read_pair->Clear_Bad_Fds();
		write_pair->Clear_Bad_Fds();
		return;
	}
	active_count -= read_pair->Deliver_Fd ( active_count );
	
	if ( active_count )
		write_pair->Deliver_Fd ( active_count );
}



KITS::OS::osSocket::~osSocket()
{
	Close_Socket();

	delete socket_address;
}

KITS::OS::osSocket::osSocket ( KITS::SOCKETS::Socket* _socket )
	: osFd ()
	, socket_fd       ( 0 )
	, socket_neuron   ( _socket )
	, socket_address  ( nullptr )
{}

KITS::OS::osSocket::osSocket ( KITS::SOCKETS::Socket* _socket, int _fd, osSocketAddress* _address )
	: osFd ()
	, socket_fd       ( _fd )
	, socket_neuron   ( _socket )
	, socket_address  ( _address )
{}

SOCKET KITS::OS::osSocket::osFd_Descriptor ( void )
{
	return socket_fd;
}

#ifdef _WIN32

void KITS::OS::osSocket::Close_Socket ( void )
{
	if ( socket_fd != INVALID_SOCKET )
		::closesocket ( socket_fd );
	socket_fd = INVALID_SOCKET;
}

void KITS::OS::osSocket::Build_Socket ( int _sockType )
{
	if ( !socket_address )
	{
		socket_neuron->config_gene->Trait_Tap ( "node", "127.0.0.1:0" );
		socket_address = new osSocketAddress ( socket_neuron->config_gene );
	}
	socket_fd = ::socket ( AF_INET, _sockType, 0 );
	u_long enable = 1;
	ioctlsocket(socket_fd, FIONBIO, &enable);
}

void KITS::OS::osSocket::Bind_Socket(bool _reuseAddr)
{
	int optValue = _reuseAddr ? 1 : 0;

	int length = static_cast<int>(socket_address->Length());
	::setsockopt (socket_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&optValue, sizeof(optValue));

	::bind        ( socket_fd, socket_address->Get(), length );
	::getsockname ( socket_fd, socket_address->Get(), &length );
}

#else

void KITS::OS::osSocket::Close_Socket ( void )
{
	if ( socket_fd )
		::close ( socket_fd );
}

void KITS::OS::osSocket::Build_Socket ( int _sockType )
{
	if ( !socket_address )
	{
		socket_neuron->config_gene->Trait_Tap ( "node", "127.0.0.1:0" );
		socket_address = new osSocketAddress ( socket_neuron->config_gene );
	}
	socket_fd = ::socket ( AF_INET, _sockType, 0 );
	::fcntl ( socket_fd, F_SETFL, O_NONBLOCK | ::fcntl ( socket_fd, F_GETFL, 0 ) );
}

void KITS::OS::osSocket::Bind_Socket ( bool _reuseAddr )
{
	int optValue =	_reuseAddr ? 1 : 0;
	::socklen_t  length = static_cast<::socklen_t> ( socket_address->Length() );

	::setsockopt  ( socket_fd, SOL_SOCKET, SO_REUSEADDR, &optValue, sizeof ( int ) );
	::bind        ( socket_fd, socket_address->Get(), length );
	::getsockname ( socket_fd, socket_address->Get(), &length );
}


#endif



KITS::OS::osSocketAddress::~osSocketAddress()
{
	delete sockAddr;
	delete ip_string;
}

KITS::OS::osSocketAddress::osSocketAddress ( void )
	: sockAddr  ( new sockaddr_in() )
	, ip_string ( new XEPL::String() )
{
	sockAddr->sin_family  = AF_INET;
	sockAddr->sin_port = 0;
	sockAddr->sin_addr.s_addr = 0;
	::memset ( sockAddr->sin_zero, 0, sizeof ( sockAddr->sin_zero ) );
}

KITS::OS::osSocketAddress::osSocketAddress ( XEPL::Gene* _config )
	: sockAddr  ( new sockaddr_in() )
	, ip_string ( new XEPL::String() )
{
	sockAddr->sin_family  = AF_INET;
	sockAddr->sin_port = 0;
	sockAddr->sin_addr.s_addr = 0;
	memset ( sockAddr->sin_zero, 0, sizeof ( sockAddr->sin_zero ) );

	XEPL::Cord* node = _config->Trait_Raw ( "node" );

	XEPL::String host(node);
	if ( !node )
		Hostname ( &host );

	XEPL::String service;
	XEPL::Split_ch_rhs ( &host, ':', &service );

	if ( host.empty() )
		host.assign ( "127.0.0.1" );

	::in_addr os_in_addr;

	if ( ::inet_pton ( AF_INET, host.c_str(), &os_in_addr ) == 1 )
	{
		sockAddr->sin_family = AF_INET;
		sockAddr->sin_port  = htons ( static_cast<int> ( std::strtol ( service.c_str(), nullptr, 0 ) ) );
		sockAddr->sin_addr = os_in_addr;
		return;
	}

	::hostent* host_entry = ::gethostbyname ( host.c_str() );

	if ( host_entry )
	{
		::in_addr** addrList = reinterpret_cast<::in_addr**> ( host_entry->h_addr_list );
		sockAddr->sin_family = AF_INET;
		sockAddr->sin_port  = htons ( std::strtol ( service.c_str(), nullptr, 0 ) );
		::memcpy ( &sockAddr->sin_addr, *addrList, sizeof ( ::in_addr ) );
		return;
	}
}

long KITS::OS::osSocketAddress::Length ( void )
{
	return sizeof ( ::sockaddr );
}

long KITS::OS::osSocketAddress::Port ( void )
{
	return static_cast<long> ( ntohs ( sockAddr->sin_port ) );
}


::sockaddr* KITS::OS::osSocketAddress::Get ( void )
{
	return reinterpret_cast<::sockaddr*> ( sockAddr );
}

XEPL::Cord* KITS::OS::osSocketAddress::IpString ( void )
{
	if ( ip_string->empty() )
	{
		ip_string->assign ( ::inet_ntoa ( sockAddr->sin_addr ) );
		ip_string->append ( ":" );
		ip_string->append ( std::to_string ( Port() ) );
	}
	return ip_string;
}

namespace KITS::OS
{
	constexpr XEPL::Text* SOC_MSG="wake";
	constexpr const int   SOC_LEN=4;
}

#ifdef _WIN32
void KITS::OS::osSocketAddress::Hostname ( XEPL::String* _hostname )
{
	if ( !_hostname )
		return;

	char selfNode[NI_MAXHOST] = {0};

	::gethostname ( selfNode, sizeof ( selfNode ) );
	_hostname->assign ( selfNode );
}
#elif __linux__
void KITS::OS::osSocketAddress::Hostname ( XEPL::String* _hostname )
{
	if ( !_hostname )
		return;

	char selfNode[_SC_HOST_NAME_MAX] = {0};
	::gethostname ( selfNode, sizeof ( selfNode ) );
	_hostname->assign ( selfNode );
}
#else
void KITS::OS::osSocketAddress::Hostname ( XEPL::String* _hostname )
{
	if ( !_hostname )
		return;

	char selfNode[_POSIX_HOST_NAME_MAX] = {0};
	::gethostname ( selfNode, sizeof ( selfNode ) );
	_hostname->assign ( selfNode );
}
#endif



KITS::OS::osSocketControl::osSocketControl ( KITS::SOCKETS::SocketControl* _socket )
	: osSocket       ( _socket )
	, socket_control ( _socket )
{
	Build_Socket ( SOCK_DGRAM );
	Bind_Socket  ( false );
}

void KITS::OS::osSocketControl::Send()
{
	osSocketAddress* addr = socket_control->control_socket->socket_address;

	::socklen_t len = static_cast<::socklen_t> ( addr->Length() );
	::sendto ( socket_fd, SOC_MSG, SOC_LEN, 0, addr->Get(), len );
}

bool KITS::OS::osSocketControl::Fd_Receive()
{
	char  recv_msg[SOC_LEN]  = {0};
	osSocketAddress* addr    = socket_control->control_socket->socket_address;

	::socklen_t len = static_cast<::socklen_t>( addr->Length() );

	ssize_t received = 0;
	do received = ::recvfrom ( socket_fd, &recv_msg, SOC_LEN, 0, addr->Get(), &len );
	while (received > 0);
	

	return true;
}

KITS::OS::osFdHandler KITS::OS::osSocketControl::osFd_OnFdRead()
{
	return ( osFdHandler )&osSocketControl::Fd_Receive;
}



KITS::OS::osTcpSocket::osTcpSocket ( KITS::SOCKETS::SocketTcp* _tcp )
	: osSocket    ( _tcp )
	, socket_tcp  ( _tcp )
{}

KITS::OS::osTcpSocket::osTcpSocket ( SOCKETS::SocketTcp* _tcp, int _fd, osSocketAddress* _address )
	: osSocket   ( _tcp, _fd, _address )
	, socket_tcp ( nullptr )
{}

void KITS::OS::osTcpSocket::Create_Tcp_Socket ( void )
{
	Build_Socket ( SOCK_STREAM );
}

bool KITS::OS::osTcpSocket::Read_Available ( void )
{
	if ( !socket_tcp )
		return false;

	const int  read_length = 8*1024-XEPL::Memory::overHead;
	char read_chars[read_length];

	while ( true )
	{
		int  recv_int = ( int )recv ( socket_fd, read_chars, read_length, 0 );
		if ( recv_int > 0 )
		{
			Receive_From_Fd ( read_chars, recv_int );

			if( recv_int == read_length )
				continue;

			eReceived->Trigger ( receiving_gene );
			receiving_gene->Release();
			receiving_gene = nullptr;

			return true;
		}

		if ( recv_int ==  0 )
		{
			socket_tcp->Closed_Socketman_Read();
			return true;
		}
		if ( errno == EAGAIN )
			return true;
	}
	return false;
}

bool KITS::OS::osTcpSocket::Send_Backpressure ( void )
{
	if ( Has_Backpressure() )
	{
		XEPL::MutexScope lock_backpressure ( fd_mutex );

		XEPL::Text* payload_chars = backpressure_wire->wire_string->data();
		size_t      send_length   = backpressure_wire->wire_string->size();

		int  send_int = ( int )::send ( socket_fd, payload_chars, send_length, 0 );
		if ( send_int == -1 )
		{
			if ( errno == EAGAIN )
			{
				errno = 0;
				return true;
			}
			socket_tcp->Closed_Socket_Write();
			return false;
		}
		if ( backpressure_wire->Expire ( send_int ) )
		{
			backpressure_wire->Release();
			backpressure_wire = nullptr;
			return false;
		}
		return true;
	}
	return false;
}

void KITS::OS::osTcpSocket::Send_Data ( XEPL::Wire* _locked_wire )
{
	XEPL::Text*   payload = _locked_wire->wire_string->data();
	size_t payload_length = _locked_wire->wire_string->length();

	int  num_bytes_sent = ( int )::send ( socket_fd, payload, payload_length, 0 );
	if ( num_bytes_sent > 0 )
	{
		_locked_wire->wire_string->erase ( 0, num_bytes_sent );
		return;
	}
	if ( errno == EAGAIN )
	{
		errno = 0;
		Add_Backpressure ( _locked_wire );

		XEPL::Spike* spike = new XEPL::Spike ( this );
		socket_manager->write_axon->Trigger ( spike );
		spike->Release();
	}
	else
		socket_tcp->Closed_Socket_Write();
}



KITS::OS::osTcpServer::~osTcpServer()
{}

KITS::OS::osTcpServer::osTcpServer ( KITS::SOCKETS::TcpServer* _server )
	: osTcpSocket ( _server )
	, tcp_server  ( _server )
{}

void KITS::OS::osTcpServer::Listen_For_Connections ( long _backlog )
{
	Bind_Socket ( true );
	::listen ( socket_fd, static_cast<int> ( _backlog ) );
}

bool KITS::OS::osTcpServer::Fd_Is_Connecting ( void )
{
	osSocketAddress* socketAddress = new osSocketAddress();
	::socklen_t addrLen = static_cast<::socklen_t> ( socketAddress->Length() );

	int socket = ::accept ( socket_fd, socketAddress->Get(), &addrLen );
	if ( socket == -1 )
	{
		delete socketAddress;
		return true;
	}
	return tcp_server->SocketMan_Connecting ( socket, socketAddress );
}


void KITS::SOCKETS::Keyword_SocketMan ( XEPL::Cortex* _cortex )
{
	_cortex->Register_Keyword ( "SocketMan", [] ( XEPL::Neuron* _neuron, XEPL::Gene* _call_gene, XEPL::String* )
	{
		XEPL::DuplicateTraits duplicate ( _call_gene );

		duplicate.gene->Trait_Tap ( "name", "SocketMan" );

		if ( _call_gene->Has_Content() )
			XEPL::Script ( _neuron, _call_gene );

		( new SocketMan ( _neuron, duplicate.gene ) )->Start_Lobe();
	} );
}

void KITS::SOCKETS::Keyword_Tcpserver ( XEPL::Cortex* _cortex )
{
	_cortex->Register_Keyword ( "TcpServer", [] ( XEPL::Neuron* _neuron, XEPL::Gene* _call_gene, XEPL::String* )
	{
		XEPL::DuplicateTraits duplicate ( _call_gene );

		duplicate.gene->Trait_Tap ( "name", "TcpServer" );

		if ( duplicate.gene->Has_Content() )
			XEPL::Script ( _neuron, duplicate.gene );

		new TcpServer ( _neuron,  duplicate.gene );
	} );
}

void KITS::SOCKETS::Register_Socket_Kit( XEPL::Cortex* _cortex )
{
	Keyword_SocketMan ( _cortex );
	Keyword_Tcpserver ( _cortex );
}
