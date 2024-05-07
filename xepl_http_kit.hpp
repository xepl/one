// SPDX: AGPL-3.0-only
/* XEPL Operating Environment - Copyright (c) 2024 Keith Edwin Robbins
	Project Name: XEPL Operating Environment
	File Name:    xepl_http_kit.hpp
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

namespace KITS::HTTP
{
	void Keyword_HttpServer ( XEPL::Cortex* _cortex );
	void Register_Http_Kit  ( XEPL::Cortex* _cortex );
}

namespace KITS::HTTP
{
	class Http;

	typedef void ( Http::*HttpScanner ) ( void  );

	class Http : public XEPL::Senson
	{
		virtual void Senson_Closed  ( void ) override;
		virtual void Senson_Scan    ( void ) override;
		virtual void Http_Deliver   ( void ) = 0;
		void Header          ( XEPL::Cord*, XEPL::String* );
		void Method_Deliver  ( XEPL::Gene*, XEPL::Gene*   );
		void Scan            ( HttpScanner );
		void Scan_Payload    ( void );
	protected:
		HttpScanner    http_scanner;
		XEPL::Gene*    message_gene;
		XEPL::Axon*    request_axon;
		XEPL::Axon*    response_axon;
		XEPL::Axon*    closed_axon;
		size_t         payload_length;
		virtual ~Http ( void ) override;
	public:
		explicit Http ( XEPL::Neuron*, XEPL::Gene* );
		void Do_Next      ( HttpScanner );
		void Scan_Header  ( void );
	};

	class HttpServer : public Http
	{
		virtual void Http_Deliver( void ) override;
		void Scan_Request_Line   ( void );
		void Method_Respond      ( XEPL::Gene*, XEPL::Gene* );
	public:
		explicit HttpServer( XEPL::Neuron*, XEPL::Gene* );
	};
}

KITS::HTTP::Http::~Http()
{
	if ( message_gene )
		message_gene->Release();
}

KITS::HTTP::Http::Http ( XEPL::Neuron*  _parent, XEPL::Gene* _gene )
	: Senson             ( _parent, _gene )
	, http_scanner       ( &Http::Senson_Scan )
	, message_gene       ( nullptr )
	, request_axon       ( new XEPL::Axon ( this, "HttpRequestAxon"  ) )
	, response_axon      ( new XEPL::Axon ( this, "HttpResponseAxon" ) )
	, closed_axon        ( new XEPL::Axon ( this, "HttpClosedAxon"   ) )
	, payload_length     ( -1 )
{
	Register_Method ( "Deliver", ( XEPL::Function )&Http::Method_Deliver, nullptr );
}

void KITS::HTTP::Http::Senson_Closed()
{
	Http_Deliver();
	closed_axon->Trigger ( nullptr );
}

void KITS::HTTP::Http::Senson_Scan()
{
	( this->*http_scanner )();
}

void KITS::HTTP::Http::Do_Next ( HttpScanner _scanner )
{
	http_scanner = _scanner;
}

void KITS::HTTP::Http::Scan ( HttpScanner _scanner )
{
	http_scanner = _scanner;
	( this->*http_scanner )();
}

void KITS::HTTP::Http::Scan_Payload()
{
	if ( payload_length == 0 )
	{
		Http_Deliver();
		Senson_Scan();
		return;
	}

	if ( payload_length > 0 && senson_wire->Avail() < payload_length )
		return;

	message_gene->Append_Content ( senson_wire );
	senson_wire->Erase();
	Http_Deliver();
	Senson_Scan();
}

void KITS::HTTP::Http::Header ( XEPL::Cord* _cord, XEPL::String* _string )
{
	XEPL::Text* chars = _string->c_str();

	while ( isspace ( *chars ) )
		++chars;

	message_gene->Make_One ( "Header" )->Trait_Set ( _cord,  chars );
}

void KITS::HTTP::Http::Scan_Header()
{
	while ( 1 )
	{
		XEPL::String line;
		senson_wire->Extract_Line ( &line );

		if ( line.empty() )
			break;

		XEPL::String definition;
		XEPL::Split_ch_rhs ( &line, ':', &definition );

		Header ( &line, &definition );
	}

	Scan ( ( HttpScanner )&Http::Scan_Payload );
}

void KITS::HTTP::Http::Method_Deliver ( XEPL::Gene*, XEPL::Gene* )
{
	Http_Deliver();
}



KITS::HTTP::HttpServer::HttpServer ( XEPL::Neuron*  _parent, XEPL::Gene* _config )
	: Http ( _parent, _config )
{
	Register_Method ( "Respond", ( XEPL::Function )&HttpServer::Method_Respond, nullptr );

	Do_Next ( ( HttpScanner )&HttpServer::Scan_Request_Line );
}

void KITS::HTTP::HttpServer::Http_Deliver()
{
	if ( message_gene )
	{
		XEPL::String content;
		if ( message_gene->Copy_Content(&content) )
		{
			XEPL::String length;
			length.assign ( std::to_string ( content.size() ) );
			Property_Set ( "content-length", &length );
		}

		request_axon->Trigger ( message_gene );
		message_gene->Release();
		message_gene = nullptr;
	}

	Do_Next ( ( HttpScanner )&HttpServer::Scan_Request_Line );
}

void KITS::HTTP::HttpServer::Scan_Request_Line()
{
	{
		XEPL::String line;
		senson_wire->Extract_Line ( &line );

		if ( line.empty() )
			return;

		line.push_back ( '\n' );

		message_gene = new XEPL::Gene ( nullptr, "HttpRequest", nullptr );
		payload_length = 0;

		XEPL::String request ( line ), uri, version;
		XEPL::Split_ch_rhs ( &request, ' ', &uri );
		XEPL::Split_ch_rhs ( &uri, ' ', &version );

		message_gene->Trait_Set ( "request", request.c_str() );
		message_gene->Trait_Set ( "uri",     uri.c_str() );
		message_gene->Trait_Set ( "version", version.c_str() );
	}
	Scan_Header();
}

void KITS::HTTP::HttpServer::Method_Respond ( XEPL::Gene* _config, XEPL::Gene* )
{
	XEPL::Gene* response_gene = new XEPL::Gene ( nullptr, "Response", nullptr );

	XEPL::String* response_string = response_gene->Make_Content();

	response_string->append ( "HTTP/1.1" );
	response_string->push_back (  ' ' );
	response_string->append ( _config->Trait_Default ( "statusCode", "200" ) );
	response_string->push_back (  ' ' );
	response_string->append ( _config->Trait_Default ( "reason", "OK" ) );
	response_string->push_back ( '\n' );

	XEPL::String message_body;
	message_body.reserve ( 4072 );

	XEPL::tlsLobe->output_string = &message_body;

	_config->Copy_Content(&message_body);

	if ( _config->inner_genes )
		Process_Inner_Genes ( _config );

	response_string->append ( "content-length: " ).append ( std::to_string (message_body.size()) ).push_back ( '\n' );
	if ( auto content_type = _config->Trait_Raw("contents") )
		response_string->append ( "content-type: " ).append ( *content_type ).push_back ( '\n' );

	response_string->push_back ( '\n' );
	response_string->append ( message_body );

	response_axon->Trigger ( response_gene );

	response_gene->Release();

	XEPL::tlsLobe->output_string = nullptr;
}


void KITS::HTTP::Keyword_HttpServer ( XEPL::Cortex* _cortex )
{
	_cortex->Register_Keyword ( "HttpServer", [] ( XEPL::Neuron* _neuron, XEPL::Gene* _call_gene, XEPL::String* )
	{
		if ( !_neuron || !_call_gene )
			return;

		if ( _call_gene->Has_Content() )
			XEPL::Script ( _neuron, _call_gene );

		KITS::HTTP::HttpServer* server = new KITS::HTTP::HttpServer ( _neuron, _call_gene );
		server->Process_Inner_Genes ( _call_gene );
	} );
}

void KITS::HTTP::Register_Http_Kit( XEPL::Cortex* _cortex )
{
	Keyword_HttpServer ( _cortex );
}
