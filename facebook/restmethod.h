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

#include "common.h"

class RestMethodPrivate;

/**
	@author 
*/
class RestMethod{
private:
    RestMethod(string, int=0, bool=false, bool=true);

    ~RestMethod();
    
    RestMethodPrivate* const d;

public:
    // Authentication
    static const RestMethod AUTH_CREATE_TOKEN;
    static const RestMethod AUTH_GET_SESSION;
    // FQL Query
    static const RestMethod FQL_QUERY;
    // Events
    static const RestMethod EVENTS_GET;
    static const RestMethod EVENTS_GET_MEMBERS;
    // Friends
    static const RestMethod FRIENDS_GET;
    static const RestMethod FRIENDS_GET_APP_USERS;
    static const RestMethod FRIENDS_GET_REQUESTS;
    static const RestMethod FRIENDS_ARE_FRIENDS;
    // Users
    static const RestMethod USERS_GET_INFO;
    static const RestMethod USERS_GET_LOGGED_IN_USER;
    static const RestMethod USERS_IS_APP_ADDED;
    // Photos
    static const RestMethod PHOTOS_GET;
    static const RestMethod PHOTOS_GET_ALBUMS;
    static const RestMethod PHOTOS_GET_TAGS;
    // PhotoUploads
    static const RestMethod PHOTOS_CREATE_ALBUM;
    static const RestMethod PHOTOS_ADD_TAG;
    static const RestMethod PHOTOS_UPLOAD;
    // Notifications
    static const RestMethod NOTIFICATIONS_GET;
    static const RestMethod NOTIFICATIONS_SEND;
    static const RestMethod NOTIFICATIONS_SEND_REQUEST;
    // Groups
    static const RestMethod GROUPS_GET;
    static const RestMethod GROUPS_GET_MEMBERS;
    // FBML
    static const RestMethod PROFILE_SET_FBML;
    static const RestMethod PROFILE_GET_FBML;
    static const RestMethod FBML_REFRESH_REF_URL;
    static const RestMethod FBML_REFRESH_IMG_SRC;
    // Feed
    static const RestMethod FEED_PUBLISH_ACTION_OF_USER;
    static const RestMethod FEED_PUBLISH_STORY_TO_USER;
    
    int numParams() const;
    string methodName() const;
    bool takesFile() const;
    int numTotalParams() const;
    bool requiresSession() const;
    bool operator==(const RestMethod &) const;
};
