/*******************************************************************************
 * libretroshare/src/services: p3gxsforums.h                                   *
 *                                                                             *
 * libretroshare: retroshare core library                                      *
 *                                                                             *
 * Copyright (C) 2012-2014  Robert Fernie <retroshare@lunamutt.com>            *
 * Copyright (C) 2018-2021  Gioacchino Mazzurco <gio@eigenlab.org>             *
 * Copyright (C) 2019-2021  Asociación Civil Altermundi <info@altermundi.net>  *
 *                                                                             *
 * This program is free software: you can redistribute it and/or modify        *
 * it under the terms of the GNU Lesser General Public License as              *
 * published by the Free Software Foundation, either version 3 of the          *
 * License, or (at your option) any later version.                             *
 *                                                                             *
 * This program is distributed in the hope that it will be useful,             *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                *
 * GNU Lesser General Public License for more details.                         *
 *                                                                             *
 * You should have received a copy of the GNU Lesser General Public License    *
 * along with this program. If not, see <https://www.gnu.org/licenses/>.       *
 *                                                                             *
 *******************************************************************************/
#pragma once

#include <map>
#include <string>

#include "retroshare/rsgxsforums.h"
#include "gxs/rsgenexchange.h"
#include "retroshare/rsgxscircles.h"
#include "util/rstickevent.h"
#include "util/rsdebug.h"

#ifdef RS_DEEP_FORUMS_INDEX
#include "deep_search/forumsindex.hpp"
#endif


class p3GxsForums: public RsGenExchange, public RsGxsForums, public p3Config,
	public RsTickEvent	/* only needed for testing - remove after */
{
public:
	p3GxsForums(
	        RsGeneralDataService* gds, RsNetworkExchangeService* nes, RsGixs* gixs);

    virtual RsServiceInfo getServiceInfo() override;
    virtual void service_tick() override;

protected:
    virtual void notifyChanges(std::vector<RsGxsNotify*>& changes) override;
	/// Overloaded from RsTickEvent.
    virtual void handle_event(uint32_t event_type, const std::string &elabel) override;

    virtual RsSerialiser* setupSerialiser() override;                            // @see p3Config::setupSerialiser()
    virtual bool saveList(bool &cleanup, std::list<RsItem *>&saveList) override; // @see p3Config::saveList(bool &cleanup, std::list<RsItem *>&)
    virtual bool loadList(std::list<RsItem *>& loadList) override;               // @see p3Config::loadList(std::list<RsItem *>&)

    virtual bool service_checkIfGroupIsStillUsed(const RsGxsGrpMetaData& meta) override;
    virtual rstime_t service_getLastGroupSeenTs(const RsGxsGroupId&) override;
public:
	/// @see RsGxsForums::createForumV2
	bool createForumV2(
	        const std::string& name, const std::string& description,
	        const RsGxsId& authorId = RsGxsId(),
	        const std::set<RsGxsId>& moderatorsIds = std::set<RsGxsId>(),
	        RsGxsCircleType circleType = RsGxsCircleType::PUBLIC,
	        const RsGxsCircleId& circleId = RsGxsCircleId(),
	        RsGxsGroupId& forumId = RS_DEFAULT_STORAGE_PARAM(RsGxsGroupId),
	        std::string& errorMessage = RS_DEFAULT_STORAGE_PARAM(std::string)
	        ) override;

	/// @see RsGxsForums::createPost
	bool createPost(
	        const RsGxsGroupId&   forumId,
	        const std::string&    title,
	        const std::string&    mBody,
	        const RsGxsId&        authorId,
	        const RsGxsMessageId& parentId = RsGxsMessageId(),
	        const RsGxsMessageId& origPostId = RsGxsMessageId(),
	        RsGxsMessageId&       postMsgId = RS_DEFAULT_STORAGE_PARAM(RsGxsMessageId),
	        std::string&          errorMessage     = RS_DEFAULT_STORAGE_PARAM(std::string)
	        ) override;

	/// @see RsGxsForums::createForum @deprecated
	RS_DEPRECATED_FOR(createForumV2)
    virtual bool createForum(RsGxsForumGroup& forum) override;

	/// @see RsGxsForums::createMessage  @deprecated
	RS_DEPRECATED_FOR(createPost)
    virtual bool createMessage(RsGxsForumMsg& message) override;

	/// @see RsGxsForums::editForum
	virtual bool editForum(RsGxsForumGroup& forum) override;

	/// @see RsGxsForums::getForumsSummaries
    virtual bool getForumsSummaries(std::list<RsGroupMetaData>& forums) override;

	/// @see RsGxsForums::getForumsInfo
	virtual bool getForumsInfo(
	        const std::list<RsGxsGroupId>& forumIds,
            std::vector<RsGxsForumGroup>& forumsInfo ) override;

    /// Implementation of @see RsGxsForums::getForumStatistics
    bool getForumStatistics(const RsGxsGroupId& ForumId,RsGxsForumStatistics& stat) override;
    bool getForumGroupStatistics(const RsGxsGroupId& ForumId,GxsGroupStatistic& stat) override;

    /// Implementation of @see RsGxsForums::getForumServiceStatistics
	bool getForumServiceStatistics(GxsServiceStatistic& stat) override;

	/// @see RsGxsForums::getForumMsgMetaData
    virtual bool getForumMsgMetaData(const RsGxsGroupId& forumId, std::vector<RsMsgMetaData>& msg_metas)  override;

    /// @see RsGxsForums::getForumPostsHierarchy
    virtual bool getForumPostsHierarchy(const RsGxsForumGroup& group,
                                        std::vector<ForumPostEntry>& vect,
                                        std::map<RsGxsMessageId,std::vector<std::pair<time_t, RsGxsMessageId> > >& post_versions) override;

	/// @see RsGxsForums::getForumContent
	virtual bool getForumContent(
	        const RsGxsGroupId& forumId,
	        const std::set<RsGxsMessageId>& msgs_to_request,
            std::vector<RsGxsForumMsg>& msgs ) override;

	/// @see RsGxsForums::markRead
    virtual bool markRead(const RsGxsGrpMsgIdPair& messageId, bool read) override;

    /// @see RsGxsForums::updateReputationLevel
    virtual void updateReputationLevel(uint32_t forum_group_sign_flags,ForumPostEntry& e) const override;

    /// @see RsGxsForums::subscribeToForum
	virtual bool subscribeToForum( const RsGxsGroupId& forumId,
                                   bool subscribe ) override;

	/// @see RsGxsForums
	bool exportForumLink(
	        std::string& link, const RsGxsGroupId& forumId,
	        bool includeGxsData = true,
	        const std::string& baseUrl = DEFAULT_FORUM_BASE_URL,
	        std::string& errMsg = RS_DEFAULT_STORAGE_PARAM(std::string)
	        ) override;

	/// @see RsGxsForums
	bool importForumLink(
	        const std::string& link,
	        RsGxsGroupId& forumId = RS_DEFAULT_STORAGE_PARAM(RsGxsGroupId),
	        std::string& errMsg = RS_DEFAULT_STORAGE_PARAM(std::string)
	        ) override;

	/// @see RsGxsForums
	std::error_condition getChildPosts(
	        const RsGxsGroupId& forumId, const RsGxsMessageId& parentId,
	        std::vector<RsGxsForumMsg>& childPosts ) override;

	/// @see RsGxsForums
	std::error_condition setPostKeepForever(
	        const RsGxsGroupId& forumId, const RsGxsMessageId& postId,
	        bool keepForever ) override;

	/// @see RsGxsForums
	std::error_condition getContentSummaries(
	        const RsGxsGroupId& forumId,
	        const std::set<RsGxsMessageId>& contentIds,
	        std::vector<RsMsgMetaData>& summaries ) override;

	/// @see RsGxsForums
	std::error_condition distantSearchRequest(
	        const std::string& matchString, TurtleRequestId& searchId ) override;

	/// @see RsGxsForums
	std::error_condition localSearch(
	        const std::string& matchString,
	        std::vector<RsGxsSearchResult>& searchResults ) override;

#ifdef RS_DEEP_FORUMS_INDEX
	/// @see RsNxsObserver
	std::error_condition handleDistantSearchRequest(
	        rs_view_ptr<uint8_t> requestData, uint32_t requestSize,
	        rs_owner_ptr<uint8_t>& resultData, uint32_t& resultSize ) override;

	/// @see RsNxsObserver
	std::error_condition receiveDistantSearchResult(
	        const TurtleRequestId requestId,
	        rs_owner_ptr<uint8_t>& resultData, uint32_t& resultSize ) override;
#endif

	std::error_condition requestSynchronization() override;

    /// implementation of rsGxsGorums
    ///
	bool getGroupData(const uint32_t &token, std::vector<RsGxsForumGroup> &groups) override;
	bool getMsgData(const uint32_t &token, std::vector<RsGxsForumMsg> &msgs) override;
	void setMessageReadStatus(uint32_t& token, const RsGxsGrpMsgIdPair& msgId, bool read) override;
	bool createGroup(uint32_t &token, RsGxsForumGroup &group) override;
	bool createMsg(uint32_t &token, RsGxsForumMsg &msg) override;
	bool updateGroup(uint32_t &token, const RsGxsForumGroup &group) override;

	bool getMsgMetaData(const uint32_t &token, GxsMsgMetaMap& msg_metas) ;

protected:
#ifdef RS_DEEP_FORUMS_INDEX
	/** Internal usage
	 * @param[in] publicOnly if true is passed only results pertaining to
	 * publicly shared forums are returned
	 */
	std::error_condition prepareSearchResults(
	        const std::string& matchString, bool publicOnly,
	        std::vector<RsGxsSearchResult>& searchResults );
#endif //def RS_DEEP_FORUMS_INDEX

private:

    static uint32_t forumsAuthenPolicy();

    void computeMessagesHierarchy(const RsGxsForumGroup& forum_group,
                                  const std::vector<RsMsgMetaData>& msgs_metas_array,
                                  std::vector<ForumPostEntry>& posts,
                                  std::map<RsGxsMessageId,std::vector<std::pair<time_t,RsGxsMessageId> > >& mPostVersions );

    virtual bool generateDummyData();

    std::string genRandomId();

    void 	dummy_tick();

    bool generateMessage(uint32_t &token, const RsGxsGroupId &grpId,
                         const RsGxsMessageId &parentId, const RsGxsMessageId &threadId);

    bool generateGroup(uint32_t &token, std::string groupName);

	class ForumDummyRef
	{
		public:
		ForumDummyRef() { return; }
		ForumDummyRef(const RsGxsGroupId &grpId, const RsGxsMessageId &threadId, const RsGxsMessageId &msgId)
		:mGroupId(grpId), mThreadId(threadId), mMsgId(msgId) { return; }

		RsGxsGroupId mGroupId;
		RsGxsMessageId mThreadId;
		RsGxsMessageId mMsgId;
	};

	uint32_t mGenToken;
	bool mGenActive;
	int mGenCount;
	std::vector<ForumDummyRef> mGenRefs;
	RsGxsMessageId mGenThreadId;
    std::map<RsGxsGroupId,rstime_t> mKnownForums ;
	
	RsMutex mKnownForumsMutex;

#ifdef RS_DEEP_FORUMS_INDEX
	DeepForumsIndex mDeepIndex;
#endif
};
