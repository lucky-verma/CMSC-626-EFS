/*
 * copyright: 2016
 * name : Francis Banyikwa
 * email: mhogomchungu@gmail.com
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef NetworkAccessManager_H
#define NetworkAccessManager_H

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

#include <QEventLoop>
#include <QTimer>

#include <vector>
#include <functional>
#include <utility>
#include <memory>

class NetworkAccessManagerTimeOutManager : public QObject
{
	Q_OBJECT
public:
	NetworkAccessManagerTimeOutManager( std::function< bool( QNetworkReply * ) > e,
					    std::function< void() > s,
					    QNetworkReply * m,
					    int w,
					    QObject * n ) :
		m_object( n ),
		m_reply( m ),
		m_cancel( std::move( e ) ),
		m_timeout( std::move( s ) )
	{
		connect( &m_timer,SIGNAL( timeout() ),
			 this,SLOT( timeout() ),Qt::QueuedConnection ) ;

		m_timer.start( 1000 * w ) ;
	}
private slots:
	void timeout()
	{
		m_timer.stop() ;

		disconnect( m_object,SIGNAL( finished( QNetworkReply * ) ),
			    this,SLOT( networkReply( QNetworkReply * ) ) ) ;

		m_cancel( m_reply ) ;

		m_timeout() ;

		this->deleteLater() ;
	}
	void networkReply( QNetworkReply * e )
	{
		if( e == m_reply ){

			m_timer.stop() ;
			this->deleteLater() ;
		}
	}
private:
	QObject * m_object ;
	QNetworkReply * m_reply ;
	QTimer m_timer ;
	std::function< bool( QNetworkReply * ) > m_cancel ;
	std::function< void() > m_timeout ;
} ;

class NetworkAccessManager : public QObject
{
	Q_OBJECT
public:
	using NetworkReply = std::unique_ptr< QNetworkReply,void( * )( QNetworkReply * ) > ;
	using function_t   = std::function< void( QNetworkReply& ) > ;
private:
	using entries_t = std::vector< std::tuple< QNetworkReply *,bool,function_t > > ;
	entries_t m_entries ;
	QNetworkAccessManager m_manager ;

	using position_t = decltype( m_entries.size() ) ;
public:
	NetworkAccessManager()
	{
		connect( &m_manager,SIGNAL( finished( QNetworkReply * ) ),
			 this,SLOT( networkReply( QNetworkReply * ) ),Qt::QueuedConnection ) ;
	}
	QNetworkAccessManager& QtNAM()
	{
		return m_manager ;
	}
	QNetworkReply * get( int timeOut,const QNetworkRequest& r,function_t f,
						 std::function< void() > m = [](){} )
	{
		auto s = m_manager.get( r ) ;

		m_entries.emplace_back( s,true,std::move( f ) ) ;

		this->setTimeOut( timeOut,s,std::move( m ) ) ;

		return s ;
	}
	NetworkReply get( int timeOut,const QNetworkRequest& r,std::function< void() > m = [](){} )
	{
		QNetworkReply * q ;

		QEventLoop s ;

		auto a = m_manager.get( r ) ;

		auto function = [ & ]( QNetworkReply& e ){ q = std::addressof( e ) ; s.quit() ;	} ;

		m_entries.emplace_back( std::make_tuple( a,false,std::move( function ) ) ) ;

		this->setTimeOut( timeOut,a,std::move( m ) ) ;

		s.exec() ;

		return { q,[]( QNetworkReply * e ){ e->deleteLater() ; } } ;
	}
	template< typename T >
	QNetworkReply * post( int timeOut,const QNetworkRequest& r,const T& e,function_t f,
			      std::function< void() > m = [](){} )
	{
		auto s = m_manager.post( r,e ) ;

		m_entries.emplace_back( std::make_tuple( s,true,std::move( f ) ) ) ;

		this->setTimeOut( timeOut,s,std::move( m ) ) ;

		return s ;
	}
	template< typename T >
	NetworkReply post( int timeOut,const QNetworkRequest& r,const T& e,
			   std::function< void() > m = [](){} )
	{
		QNetworkReply * q ;

		QEventLoop s ;

		auto function = [ & ]( QNetworkReply& e ){ q = std::addressof( e ) ; s.quit() ;	} ;

		auto a = m_manager.post( r,e ) ;

		m_entries.emplace_back( std::make_tuple( a,false,std::move( function ) ) ) ;

		this->setTimeOut( timeOut,a,std::move( m ) ) ;

		s.exec() ;

		return { q,[]( QNetworkReply * e ){ e->deleteLater() ; } } ;
	}
	QNetworkReply * head( int timeOut,const QNetworkRequest& r,function_t f,
			      std::function< void() > m = [](){} )
	{
		auto s = m_manager.head( r ) ;

		m_entries.emplace_back( std::make_tuple( s,true,std::move( f ) ) ) ;

		this->setTimeOut( timeOut,s,std::move( m ) ) ;

		return s ;
	}
	NetworkReply head( int timeOut,const QNetworkRequest& r,std::function< void() > m = [](){} )
	{
		QNetworkReply * q ;

		QEventLoop s ;

		auto function = [ & ]( QNetworkReply& e ){ q = std::addressof( e ) ; s.quit() ;	} ;

		auto a = m_manager.head( r ) ;

		m_entries.emplace_back( std::make_tuple( a,false,std::move( function ) ) ) ;

		this->setTimeOut( timeOut,a,std::move( m ) ) ;

		s.exec() ;

		return { q,[]( QNetworkReply * e ){ e->deleteLater() ; } } ;
	}
	bool cancel( QNetworkReply * e )
	{
		return this->find_network_reply( e,[ this ]( QNetworkReply& e,position_t s ){

			if( std::get< 1 >( m_entries[ s ] ) ){

				e.deleteLater() ;
			}

			m_entries.erase( m_entries.begin() + s ) ;

			e.close() ;
			e.abort() ;
		} ) ;
	}
private:
	void setTimeOut( int s,QNetworkReply * e,std::function< void() > m )
	{
		if( s > 0 ){

			auto a = [ this ]( QNetworkReply * e ){	return this->cancel( e ) ; } ;

			auto u = new NetworkAccessManagerTimeOutManager( std::move( a ),
									 std::move( m ),
									 e,s,&m_manager ) ;

			connect( &m_manager,SIGNAL( finished( QNetworkReply * ) ),
				 u,SLOT( networkReply( QNetworkReply * ) ),Qt::QueuedConnection ) ;
		}
	}
	bool find_network_reply( QNetworkReply * e,std::function< void( QNetworkReply&,position_t ) > function )
	{
		for( position_t s = 0 ; s < m_entries.size() ; s++ ){

			if( std::get< 0 >( m_entries[ s ] ) == e ){

				function( *e,s ) ;

				return true ;
			}
		}

		return false ;
	}
private slots:
	void networkReply( QNetworkReply * e )
	{
		this->find_network_reply( e,[ this ]( QNetworkReply& e,position_t s ){

			std::get< 2 >( m_entries[ s ] )( e ) ;

			if( std::get< 1 >( m_entries[ s ] ) ){

				e.deleteLater() ;
			}

			m_entries.erase( m_entries.begin() + s ) ;
		} ) ;
	}
};

#endif
