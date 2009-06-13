/*
Copyright © 2009 Michal Zelinka

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "restmethod.h"

class RestMethodPrivate
{
public:
    string methodName;
    int numParams;
    int maxParamsWithSession;
    bool takesFiles;
    bool requiresSession;
};

/// Initialize static members (emulate Enum)
/// @todo figure out how to hide static members from instances

// Authentication
const RestMethod RestMethod::AUTH_CREATE_TOKEN("facebook.auth.createToken", 0, false, false);
const RestMethod RestMethod::AUTH_GET_SESSION("facebook.auth.getSession", 1, false, false);
// FQL Query
const RestMethod RestMethod::FQL_QUERY("facebook.fql.query",1);
// Events
const RestMethod RestMethod::EVENTS_GET("facebook.events.get", 5);
const RestMethod RestMethod::EVENTS_GET_MEMBERS("facebook.events.getMembers", 1);
// Friends
const RestMethod RestMethod::FRIENDS_GET("facebook.friends.get");
const RestMethod RestMethod::FRIENDS_GET_APP_USERS("facebook.friends.getAppUsers");
const RestMethod RestMethod::FRIENDS_GET_REQUESTS("facebook.friends.getRequests");
const RestMethod RestMethod::FRIENDS_ARE_FRIENDS("facebook.friends.areFriends", 2);
// Users
const RestMethod RestMethod::USERS_GET_INFO("facebook.users.getInfo", 2);
const RestMethod RestMethod::USERS_GET_LOGGED_IN_USER("facebook.users.getLoggedInUser");
const RestMethod RestMethod::USERS_IS_APP_ADDED("facebook.users.isAppAdded");
// Photos
const RestMethod RestMethod::PHOTOS_GET("facebook.photos.get", 2);
const RestMethod RestMethod::PHOTOS_GET_ALBUMS("facebook.photos.getAlbums", 1);
const RestMethod RestMethod::PHOTOS_GET_TAGS("facebook.photos.getTags", 1);
// PhotoUploads
const RestMethod RestMethod::PHOTOS_CREATE_ALBUM("facebook.photos.createAlbum",3);
const RestMethod RestMethod::PHOTOS_ADD_TAG("facebook.photos.addTag", 5);
const RestMethod RestMethod::PHOTOS_UPLOAD("facebook.photos.upload", 3, true);
// Notifications
const RestMethod RestMethod::NOTIFICATIONS_GET("facebook.notifications.get");
const RestMethod RestMethod::NOTIFICATIONS_SEND("facebook.notifications.send",5);
const RestMethod RestMethod::NOTIFICATIONS_SEND_REQUEST("facebook.notifications.sendRequest",5);
// Groups
const RestMethod RestMethod::GROUPS_GET("facebook.groups.get", 1);
const RestMethod RestMethod::GROUPS_GET_MEMBERS("facebook.groups.getMembers", 1);
// FBML
const RestMethod RestMethod::PROFILE_SET_FBML("facebook.profile.setFBML", 2);
const RestMethod RestMethod::PROFILE_GET_FBML("facebook.profile.getFBML", 1);
const RestMethod RestMethod::FBML_REFRESH_REF_URL("facebook.fbml.refreshRefUrl", 1);
const RestMethod RestMethod::FBML_REFRESH_IMG_SRC("facebook.fbml.refreshImgSrc", 1);
// Feed
const RestMethod RestMethod::FEED_PUBLISH_ACTION_OF_USER("facebook.feed.publishActionOfUser", 11);
const RestMethod RestMethod::FEED_PUBLISH_STORY_TO_USER("facebook.feed.publishStoryToUser", 11);


RestMethod::RestMethod(string methodName, int maxParams, bool takesFiles, bool requiresSession) : d(new RestMethodPrivate)
{
    /// Initialize
    d->methodName = methodName;
    d->numParams = maxParams;
    //TODO d->maxParamsWithSession = maxParams + FacebookRestClient::NUM_AUTOAPPENDED_PARAMS;
    d->takesFiles = takesFiles;
    d->requiresSession = requiresSession;
}


RestMethod::~RestMethod()
{
    delete d;
}


/*!
    \fn RestMethod::methodName()
 */
string RestMethod::methodName() const
{
    return d->methodName;
}

/*!
    \fn RestMethod::numParams
 */
int RestMethod::numParams() const
{
    return d->numParams;
}


/*!
    \fn RestMethod::requiresSession()
 */
bool RestMethod::requiresSession() const
{
    return d->requiresSession;
}


/*!
    \fn RestMethod::numTotalParams()
 */
int RestMethod::numTotalParams() const
{
    return requiresSession() ? d->maxParamsWithSession : d->numParams;
}


/*!
    \fn RestMethod::takesFile()
 */
bool RestMethod::takesFile() const
{
    return d->takesFiles;
}


/*!
    \fn RestMethod::operator==(const RestMethod & method) const
 */
bool RestMethod::operator==(const RestMethod & other) const
{
    return methodName()==other.methodName();
}
