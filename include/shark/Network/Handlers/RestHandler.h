/**
 *
 *  \brief Implements a REST API for the shark probe framework.
 *
 *  \author  T. Voss
 *  \date    2011
 *
 *  \par Copyright (c) 2007-2011:
 *      Institut f&uuml;r Neuroinformatik<BR>
 *      Ruhr-Universit&auml;t Bochum<BR>
 *      D-44780 Bochum, Germany<BR>
 *      Phone: +49-234-32-25558<BR>
 *      Fax:   +49-234-32-14209<BR>
 *      eMail: Shark-admin@neuroinformatik.ruhr-uni-bochum.de<BR>
 *      www:   http://www.neuroinformatik.ruhr-uni-bochum.de<BR>
 *
 *
 *  <BR><HR>
 *  This file is part of Shark. This library is free software;
 *  you can redistribute it and/or modify it under the terms of the
 *  GNU General Public License as published by the Free Software
 *  Foundation; either version 3, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this library; if not, see <http://www.gnu.org/licenses/>.
 *  
 */
#ifndef SHARK_NETWORK_HANDLERS_REST_HANDLER_H
#define SHARK_NETWORK_HANDLERS_REST_HANDLER_H

#include <shark/Core/Logger.h>
#include <shark/Core/Probe.h>

#include <shark/Network/HttpServer.h>

#include <boost/bimap.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/noncopyable.hpp>

#include <sstream>

namespace shark {

    /**
     * \brief Implements a REST API for the shark probe framework.
     */
    class RestHandler : public HttpServer::AbstractRequestHandler {
    protected:

	/**
	 * \brief Function object for visiting registered probes and serializing
	 * them to JSON.
	 */
	struct JsonProbeVisitor {

	    /**
	     * \brief Constructs this visitor for the supplied stream.
	     */ 
	JsonProbeVisitor( std::stringstream & stream ) : m_first( true ), m_stream( stream ) {
	    m_stream << "{\"probes\": [" << std::endl;
	}

	    /**
	     * \brief D'tor, finishes up the JSON stream.
	     */
	    ~JsonProbeVisitor() {
		m_stream << "]}" << std::endl;
	    }

	    /**
	     * \brief Visitor callback.
	     */ 
	    void operator()( const ProbeManager::Path & path, const ProbeManager::ProbePtr & probe ) {
		Probe::timestamped_value_type timestampedValue = probe->timestampedValue();

		//Probe::variant_type value;
		//probe->value( value );
                
		if( !m_first )
		    m_stream << ",";
		m_stream << 
		    "{ \"name\":\"" << 
		    probe->name() << 
		    "\", \"value\":\"" << 
		    timestampedValue.first <<  
                    "\", \"timestamp\":\"" << 
		    timestampedValue.second.value()
			 << "\"}"; 

		m_first = false;
	    }

	    bool m_first; ///< Stores whether the Visitor visits the first probe.
	    std::stringstream & m_stream; ///< Reference to the target stream.
	};
    public:

	/**
	 * \brief Iterates all registered probes, serializes them and sends 
	 * the resulting JSON document to the requesting client.
	 */
	void handle( const HttpServer::request_type & request, HttpServer::connection_ptr_type connection ) {

	    if( request.method != "GET" ) {
		connection->set_status( HttpServer::connection_type::not_found );
		return;
	    }
			
	    std::stringstream ss;
	    {
		JsonProbeVisitor visitor( ss );
		shark::ProbeManager::instance().visit( 
						      boost::bind( 
								  &JsonProbeVisitor::operator(), boost::ref( visitor ), _1, _2 
								   ) 
						       );
	    }

	    HttpServer::async_server_type::response_header headers[] = {
		{"Content-Type", "application/json"},
		{"Content-Length", "0" },
	    };

	    headers[ 1 ].value = boost::lexical_cast< std::string >( ss.str().size() );

	    connection->set_status( HttpServer::connection_type::ok );
	    connection->set_headers( boost::make_iterator_range( headers, headers + 2 ) );
	    connection->write( ss.str() );
	}		
    };
}

#endif // SHARK_NETWORK_HANDLERS_REST_HANDLER_H
