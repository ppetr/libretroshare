#pragma once

#include "rsitems/rsserviceids.h"
#include "rsitems/rsitem.h"
#include "rsitems/itempriorities.h"

#include "serialiser/rsserial.h"
#include "serialiser/rstlvbase.h"
#include "serialiser/rsbaseserial.h"

#include "retroshare/rsturtle.h"
#include "retroshare/rsexpr.h"
#include "retroshare/rstypes.h"
#include "turtle/turtletypes.h"

#include "serialiser/rsserializer.h"

const uint8_t RS_TURTLE_SUBTYPE_STRING_SEARCH_REQUEST	= 0x01 ;
const uint8_t RS_TURTLE_SUBTYPE_FT_SEARCH_RESULT		= 0x02 ;
const uint8_t RS_TURTLE_SUBTYPE_OPEN_TUNNEL    			= 0x03 ;
const uint8_t RS_TURTLE_SUBTYPE_TUNNEL_OK      			= 0x04 ;
const uint8_t RS_TURTLE_SUBTYPE_FILE_REQUEST   			= 0x07 ;
const uint8_t RS_TURTLE_SUBTYPE_FILE_DATA      			= 0x08 ;
const uint8_t RS_TURTLE_SUBTYPE_REGEXP_SEARCH_REQUEST   = 0x09 ;
const uint8_t RS_TURTLE_SUBTYPE_GENERIC_DATA     		= 0x0a ;
const uint8_t RS_TURTLE_SUBTYPE_GXS_SEARCH_REQUEST      = 0x0b ;
const uint8_t RS_TURTLE_SUBTYPE_GXS_GROUP_REQUEST       = 0x0c ;
const uint8_t RS_TURTLE_SUBTYPE_FILE_MAP                = 0x10 ;
const uint8_t RS_TURTLE_SUBTYPE_FILE_MAP_REQUEST        = 0x11 ;
const uint8_t RS_TURTLE_SUBTYPE_CHUNK_CRC               = 0x14 ;
const uint8_t RS_TURTLE_SUBTYPE_CHUNK_CRC_REQUEST       = 0x15 ;
const uint8_t RS_TURTLE_SUBTYPE_GXS_GROUP_SUMMARY		= 0x16 ;
const uint8_t RS_TURTLE_SUBTYPE_GXS_GROUP_DATA   		= 0x17 ;

// const uint8_t RS_TURTLE_SUBTYPE_FILE_CRC                = 0x12 ; // unused
// const uint8_t RS_TURTLE_SUBTYPE_FILE_CRC_REQUEST        = 0x13 ;

class TurtleSearchRequestInfo ;

/***********************************************************************************/
/*                           Basic Turtle Item Class                               */
/***********************************************************************************/

class RsTurtleItem: public RsItem
{
	public:
		RsTurtleItem(uint8_t turtle_subtype) : RsItem(RS_PKT_VERSION_SERVICE,RS_SERVICE_TYPE_TURTLE,turtle_subtype) {}
};

/***********************************************************************************/
/*                           Turtle Search Item classes                            */
/*                                Specific packets                                 */
/***********************************************************************************/

// Class hierarchy is
//
//     RsTurtleItem
//         |
//         +---- RsTurtleSearchRequestItem
//         |               |
//         |               +---- RsTurtleFileSearchRequestItem
//         |               |                  |
//         |               |                  +---- RsTurtleFileSearchRequestItem
//         |               |                                    |
//         |               |                                    +---- RsTurtleStringSearchRequestItem
//         |               |                                    |
//         |               |                                    +---- RsTurtleReqExpSearchRequestItem
//         |               |
//         |               +---- RsTurtleGxsSearchRequestItem
//         |               |
//         |               +---- RsTurtleGxsGroupRequestItem
//         |
//         +---- RsTurtleSearchResultItem
//                         |
//                         +---- RsTurtleFTSearchResultItem
//                         |
//                         +---- RsTurtleGxsSearchResultItem
//                                           |
//                                           +---- RsTurtleGxsSearchResultGroupSummaryItem
//                                           |
//                                           +---- RsTurtleGxsSearchResultGroupDataItem
//

class RsTurtleSearchResultItem ;

class RsTurtleSearchRequestItem: public RsTurtleItem
{
	public:
        RsTurtleSearchRequestItem(uint32_t subtype) : RsTurtleItem(subtype), request_id(0), depth(0) { setPriorityLevel(QOS_PRIORITY_RS_TURTLE_SEARCH_REQUEST) ;}
        virtual ~RsTurtleSearchRequestItem() {}

		virtual RsTurtleSearchRequestItem *clone() const = 0 ;					// used for cloning in routing methods

		virtual void performLocalSearch(TurtleSearchRequestInfo& req,std::list<RsTurtleSearchResultItem*>&) const = 0 ;	// abstracts the search method

		virtual std::string GetKeywords() = 0;

		uint32_t request_id ; 		// randomly generated request id.
		uint16_t depth ;				// Used for limiting search depth.
};

class RsTurtleFileSearchRequestItem: public RsTurtleSearchRequestItem
{
	public:
        RsTurtleFileSearchRequestItem(uint32_t subtype) : RsTurtleSearchRequestItem(subtype) {}
        virtual ~RsTurtleFileSearchRequestItem() {}

		virtual void performLocalSearch(TurtleSearchRequestInfo& req,std::list<RsTurtleSearchResultItem*>&) const ;	// abstracts the search method
		virtual void search(std::list<TurtleFileInfo> &) const =0;
};

class RsTurtleStringSearchRequestItem: public RsTurtleFileSearchRequestItem
{
	public:
		RsTurtleStringSearchRequestItem() : RsTurtleFileSearchRequestItem(RS_TURTLE_SUBTYPE_STRING_SEARCH_REQUEST) {}
        virtual ~RsTurtleStringSearchRequestItem() {}

		std::string match_string ;	// string to match

		std::string GetKeywords() { return match_string; }

		virtual RsTurtleSearchRequestItem *clone() const { return new RsTurtleStringSearchRequestItem(*this) ; }

        void clear() { match_string.clear() ; }

	protected:
		virtual void search(std::list<TurtleFileInfo> &) const ;
		void serial_process(RsGenericSerializer::SerializeJob j,RsGenericSerializer::SerializeContext& ctx);
};

class RsTurtleRegExpSearchRequestItem: public RsTurtleFileSearchRequestItem
{
	public:
		RsTurtleRegExpSearchRequestItem() : RsTurtleFileSearchRequestItem(RS_TURTLE_SUBTYPE_REGEXP_SEARCH_REQUEST) {}
        virtual ~RsTurtleRegExpSearchRequestItem() {}

        RsRegularExpression::LinearizedExpression expr ;	// Reg Exp in linearised mode

		std::string GetKeywords()
		{
			RsRegularExpression::Expression *ex = RsRegularExpression::LinearizedExpression::toExpr(expr);
			std::string exs = ex->toStdString();
			delete ex;
			return exs;
		}

		virtual RsTurtleSearchRequestItem *clone() const { return new RsTurtleRegExpSearchRequestItem(*this) ; }
		void clear() { expr = RsRegularExpression::LinearizedExpression(); }
	protected:
		virtual void search(std::list<TurtleFileInfo> &) const ;
		void serial_process(RsGenericSerializer::SerializeJob j,RsGenericSerializer::SerializeContext& ctx);
};

class RsTurtleGxsSearchRequestItem: public RsTurtleSearchRequestItem
{
	public:
		RsTurtleGxsSearchRequestItem() : RsTurtleSearchRequestItem(RS_TURTLE_SUBTYPE_GXS_SEARCH_REQUEST) {}
        virtual ~RsTurtleGxsSearchRequestItem() {}

		std::string match_string ;	// string to match
        uint16_t service_id ;		// searvice to search

		std::string GetKeywords() { return match_string; }

		virtual void performLocalSearch(TurtleSearchRequestInfo& req,std::list<RsTurtleSearchResultItem*>&) const ;	// abstracts the search method
		virtual RsTurtleSearchRequestItem *clone() const { return new RsTurtleGxsSearchRequestItem(*this) ; }
        void clear() { match_string.clear() ; }

	protected:
		void serial_process(RsGenericSerializer::SerializeJob j,RsGenericSerializer::SerializeContext& ctx);
};

class RsTurtleGxsGroupRequestItem: public RsTurtleSearchRequestItem
{
	public:
		RsTurtleGxsGroupRequestItem() : RsTurtleSearchRequestItem(RS_TURTLE_SUBTYPE_GXS_GROUP_REQUEST) {}
        virtual ~RsTurtleGxsGroupRequestItem() {}

        uint16_t service_id ;		// searvice to search
		Sha1CheckSum hashed_group_id ;	// the group ID is hashed in order to keep it private.

		virtual RsTurtleSearchRequestItem *clone() const { return new RsTurtleGxsGroupRequestItem(*this) ; }

		virtual void performLocalSearch(TurtleSearchRequestInfo& req,std::list<RsTurtleSearchResultItem*>&) const ;	// abstracts the search method
        void clear() { hashed_group_id.clear() ; }
		std::string GetKeywords() { return hashed_group_id.toStdString(); }

	protected:
		void serial_process(RsGenericSerializer::SerializeJob j,RsGenericSerializer::SerializeContext& ctx);
};

class RsTurtleSearchResultItem: public RsTurtleItem
{
	public:
        RsTurtleSearchResultItem(uint8_t subtype) : RsTurtleItem(subtype), request_id(0), depth(0) { setPriorityLevel(QOS_PRIORITY_RS_TURTLE_SEARCH_RESULT) ;}

		TurtleSearchRequestId request_id ;	// Randomly generated request id.
		uint16_t depth ;					// The depth of a search result is obfuscated in this way:
											// 	If the actual depth is 1, this field will be 1.
											// 	If the actual depth is > 1, this field is a larger arbitrary integer.

        virtual void clear() =0;
        virtual uint32_t count() const =0;
        virtual void pop() =0;

		virtual void serial_process(RsGenericSerializer::SerializeJob j,RsGenericSerializer::SerializeContext& ctx)=0;
        virtual RsTurtleSearchResultItem *duplicate() const =0;
};

class RsTurtleFTSearchResultItem: public RsTurtleSearchResultItem
{
	public:
        RsTurtleFTSearchResultItem() : RsTurtleSearchResultItem(RS_TURTLE_SUBTYPE_FT_SEARCH_RESULT){}

		std::list<TurtleFileInfo> result ;

        void clear() { result.clear() ; }
        uint32_t count() const { return result.size() ; }
        virtual void pop() { result.pop_back() ;}
        virtual RsTurtleSearchResultItem *duplicate() const { return new RsTurtleFTSearchResultItem(*this) ; }
	protected:
		void serial_process(RsGenericSerializer::SerializeJob j,RsGenericSerializer::SerializeContext& ctx);
};

class RsTurtleGxsSearchResultGroupSummaryItem: public RsTurtleSearchResultItem
{
	public:
        RsTurtleGxsSearchResultGroupSummaryItem() : RsTurtleSearchResultItem(RS_TURTLE_SUBTYPE_GXS_GROUP_SUMMARY){}
        virtual ~RsTurtleGxsSearchResultGroupSummaryItem() {}

		std::list<TurtleGxsInfo> result ;

        void clear() { result.clear() ; }
        uint32_t count() const { return result.size() ; }
        virtual void pop() { result.pop_back() ;}
        virtual RsTurtleSearchResultItem *duplicate() const { return new RsTurtleGxsSearchResultGroupSummaryItem(*this) ; }
	protected:
		void serial_process(RsGenericSerializer::SerializeJob j,RsGenericSerializer::SerializeContext& ctx);
};
class RsTurtleGxsSearchResultGroupDataItem: public RsTurtleSearchResultItem
{
	public:
        RsTurtleGxsSearchResultGroupDataItem() : RsTurtleSearchResultItem(RS_TURTLE_SUBTYPE_GXS_GROUP_DATA){}
        virtual ~RsTurtleGxsSearchResultGroupDataItem() {}

        RsTlvBinaryData encrypted_nxs_group;	// data is encrypted with group ID.

        uint32_t count() const { return 1 ; }
        virtual void pop() { clear(); }
        void clear() { encrypted_nxs_group.TlvClear() ; }
        virtual RsTurtleSearchResultItem *duplicate() const { return new RsTurtleGxsSearchResultGroupDataItem(*this) ; }
	protected:
		void serial_process(RsGenericSerializer::SerializeJob j,RsGenericSerializer::SerializeContext& ctx);
};

/***********************************************************************************/
/*                           Turtle Tunnel Item classes                            */
/***********************************************************************************/

class RsTurtleOpenTunnelItem: public RsTurtleItem
{
	public:
        RsTurtleOpenTunnelItem() : RsTurtleItem(RS_TURTLE_SUBTYPE_OPEN_TUNNEL), request_id(0), partial_tunnel_id(0), depth(0) { setPriorityLevel(QOS_PRIORITY_RS_TURTLE_OPEN_TUNNEL) ;}

		TurtleFileHash file_hash ;	  // hash to match
		uint32_t request_id ;		  // randomly generated request id.
		uint32_t partial_tunnel_id ; // uncomplete tunnel id. Will be completed at destination.
		uint16_t depth ;				  // Used for limiting search depth.

        void clear() { file_hash.clear() ;}
	protected:
		void serial_process(RsGenericSerializer::SerializeJob j,RsGenericSerializer::SerializeContext& ctx);
};

class RsTurtleTunnelOkItem: public RsTurtleItem
{
	public:
        RsTurtleTunnelOkItem() : RsTurtleItem(RS_TURTLE_SUBTYPE_TUNNEL_OK), tunnel_id(0), request_id(0) { setPriorityLevel(QOS_PRIORITY_RS_TURTLE_TUNNEL_OK) ;}

		uint32_t tunnel_id ;		// id of the tunnel. Should be identical for a tunnel between two same peers for the same hash.
		uint32_t request_id ;	// randomly generated request id corresponding to the intial request.

        void clear() {}
	protected:
		void serial_process(RsGenericSerializer::SerializeJob j,RsGenericSerializer::SerializeContext& ctx);
};

/***********************************************************************************/
/*                           Generic turtle packets for tunnels                    */
/***********************************************************************************/

class RsTurtleGenericTunnelItem: public RsTurtleItem
{
	public:
        RsTurtleGenericTunnelItem(uint8_t sub_packet_id) : RsTurtleItem(sub_packet_id), direction(0), tunnel_id(0) { setPriorityLevel(QOS_PRIORITY_RS_TURTLE_GENERIC_ITEM);}
        virtual ~RsTurtleGenericTunnelItem() {}

		typedef uint32_t Direction ;
		static const Direction DIRECTION_CLIENT = 0x001 ;
		static const Direction DIRECTION_SERVER = 0x002 ;

		/// Does this packet stamps tunnels when it passes through ?
		/// This is used for keeping trace weither tunnels are active or not.

		virtual bool shouldStampTunnel() const = 0 ;

		/// All tunnels derived from RsTurtleGenericTunnelItem should have a tunnel id to
		/// indicate which tunnel they are travelling through.

		virtual TurtleTunnelId tunnelId() const { return tunnel_id ; }

		/// Indicate weither the packet is a client packet (goign back to the
		/// client) or a server packet (going to the server. Typically file
		/// requests are server packets, whereas file data are client packets.

		virtual Direction travelingDirection() const { return direction ; }
		virtual void setTravelingDirection(Direction d) { direction = d; }

		Direction direction ;	// This does not need to be serialised. It's only used by the client services, optionnally,
										// and is set by the turtle router according to which direction the item travels.

		uint32_t tunnel_id ;		// Id of the tunnel to travel through
};

/***********************************************************************************/
/*                           Specific Turtle Transfer items                        */
/***********************************************************************************/

// This item can be used by any service to pass-on arbitrary data into a tunnel.
//
class RsTurtleGenericDataItem: public RsTurtleGenericTunnelItem
{
	public:
        RsTurtleGenericDataItem() : RsTurtleGenericTunnelItem(RS_TURTLE_SUBTYPE_GENERIC_DATA), data_size(0), data_bytes(0) { setPriorityLevel(QOS_PRIORITY_RS_TURTLE_FILE_REQUEST);}
		virtual ~RsTurtleGenericDataItem() { if(data_bytes != NULL) free(data_bytes) ; }

		virtual bool shouldStampTunnel() const { return true ; }

		uint32_t data_size ;
		void *data_bytes ;

        void clear()
        {
            free(data_bytes) ;
            data_bytes = NULL ;
            data_size = 0;
        }
	protected:
		void serial_process(RsGenericSerializer::SerializeJob j,RsGenericSerializer::SerializeContext& ctx);
};

/***********************************************************************************/
/*                           Turtle Serialiser class                               */
/***********************************************************************************/

class RsTurtleSerialiser: public RsServiceSerializer
{
	public:
		RsTurtleSerialiser() : RsServiceSerializer(RS_SERVICE_TYPE_TURTLE) {}

		virtual RsItem *create_item(uint16_t service,uint8_t item_subtype) const;

		// This is used by the turtle router to add services to its serialiser.
		// Client services are only used for deserialising, since the serialisation is
		// performed using the overloaded virtual functions above.
		//
		void registerClientService(RsTurtleClientService *service) { _client_services.push_back(service) ; }

	private:
		std::vector<RsTurtleClientService *> _client_services ;
};

