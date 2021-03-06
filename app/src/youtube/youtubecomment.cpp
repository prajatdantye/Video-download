/*
 * Copyright (C) 2016 Stuart Howarth <showarth@marxoft.co.uk>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "youtubecomment.h"
#include "logger.h"
#include "resources.h"
#include "youtube.h"
#include <QDateTime>

YouTubeComment::YouTubeComment(QObject *parent) :
    CTComment(parent),
    m_request(0)
{
    setService(Resources::YOUTUBE);
}

YouTubeComment::YouTubeComment(const QString &id, QObject *parent) :
    CTComment(parent),
    m_request(0)
{
    setService(Resources::YOUTUBE);
    loadComment(id);
}

YouTubeComment::YouTubeComment(const QVariantMap &comment, QObject *parent) :
    CTComment(parent),
    m_request(0)
{
    setService(Resources::YOUTUBE);
    loadComment(comment);
}

YouTubeComment::YouTubeComment(const YouTubeComment *comment, QObject *parent) :
    CTComment(comment, parent),
    m_request(0),
    m_parentId(comment->parentId())
{
}

QString YouTubeComment::errorString() const {
    return m_request ? YouTube::getErrorString(m_request->result().toMap()) : QString();
}

QString YouTubeComment::parentId() const {
    return m_parentId;
}

void YouTubeComment::setParentId(const QString &i) {
    if (i != parentId()) {
        m_parentId = i;
        emit parentIdChanged();
    }
}

QYouTube::ResourcesRequest::Status YouTubeComment::status() const {
    return m_request ? m_request->status() : QYouTube::ResourcesRequest::Null;
}

void YouTubeComment::loadComment(const QString &id) {
    if (status() == QYouTube::ResourcesRequest::Loading) {
        return;
    }
    
    initRequest();
    
    QVariantMap filters;
    filters["id"] = id;
    filters["textFormat"] = "plainText";
    m_request->list("/comments", QStringList() << "snippet", filters);
    connect(m_request, SIGNAL(finished()), this, SLOT(onCommentRequestFinished()));
    emit statusChanged(status());
}

void YouTubeComment::loadComment(const QVariantMap &comment) {
    const QVariantMap snippet = comment.value("snippet").toMap();
    setBody(QString::fromUtf8(snippet.value("textDisplay").toString().toUtf8().replace("\ufeff", "").simplified()));
    setDate(QDateTime::fromString(snippet.value("publishedAt").toString(), Qt::ISODate).toString("dd MMM yyyy"));
    setId(comment.value("id").toString());
    setParentId(snippet.value("parentId").toString());
    setThumbnailUrl(snippet.value("authorProfileImageUrl").toString());
    setUserId(snippet.value("authorChannelId").toMap().value("value").toString());
    setUsername(snippet.value("authorDisplayName").toString());
    setVideoId(snippet.value("videoId").toString());
}

void YouTubeComment::loadComment(YouTubeComment *comment) {
    CTComment::loadComment(comment);
    setParentId(comment->parentId());
}

void YouTubeComment::addComment() {
    if (status() == QYouTube::ResourcesRequest::Loading) {
        return;
    }
    
    Logger::log("YouTubeComment::addComment()", Logger::MediumVerbosity);
    initRequest();
    QVariantMap resource;
    QVariantMap snippet;
    
    if (parentId().isEmpty()) {
        QVariantMap topLevelComment;
        QVariantMap topLevelSnippet;
        topLevelSnippet["textOriginal"] = body();
        topLevelComment["snippet"] = topLevelSnippet;
        snippet["channelId"] = YouTube::userId();
        snippet["topLevelComment"] = topLevelComment;
        
        if (!videoId().isEmpty()) {
            snippet["videoId"] = videoId();
        }
        
        resource["snippet"] = snippet;
        m_request->insert(resource, "/commentThreads", QStringList() << "snippet");
    }
    else {
        snippet["parentId"] = parentId();
        snippet["textOriginal"] = body();
        resource["snippet"] = snippet;
        m_request->insert(resource, "/comments", QStringList() << "snippet");
    }
        
    connect(m_request, SIGNAL(finished()), this, SLOT(onAddCommentRequestFinished()));
    emit statusChanged(status());
}

void YouTubeComment::addComment(const QVariantMap &comment) {
    loadComment(comment);
    addComment();
}

void YouTubeComment::initRequest() {
    if (!m_request) {
        m_request = new QYouTube::ResourcesRequest(this);
        m_request->setApiKey(YouTube::apiKey());
        m_request->setClientId(YouTube::clientId());
        m_request->setClientSecret(YouTube::clientSecret());
        m_request->setAccessToken(YouTube::accessToken());
        m_request->setRefreshToken(YouTube::refreshToken());
    
        connect(m_request, SIGNAL(accessTokenChanged(QString)), YouTube::instance(), SLOT(setAccessToken(QString)));
        connect(m_request, SIGNAL(refreshTokenChanged(QString)), YouTube::instance(), SLOT(setRefreshToken(QString)));
    }
}

void YouTubeComment::onCommentRequestFinished() {
    if (m_request->status() == QYouTube::ResourcesRequest::Ready) {
        loadComment(m_request->result().toMap());
    }
    
    disconnect(m_request, SIGNAL(finished()), this, SLOT(onCommentRequestFinished()));
    emit statusChanged(status());
}

void YouTubeComment::onAddCommentRequestFinished() {
    if (m_request->status() == QYouTube::ResourcesRequest::Ready) {
        const QVariantMap result = m_request->result().toMap();
        
        if (result.value("kind") == "youtube#commentThread") {
            loadComment(result.value("snippet").toMap().value("topLevelComment").toMap());
        }
        else {
            loadComment(result);
        }
        
        Logger::log("YouTubeComment::onAddCommentRequestFinished(). Comment added. ID: " + id(),
                    Logger::MediumVerbosity);
        emit YouTube::instance()->commentAdded(this);
    }
    else {
        Logger::log("YouTubeComment::onAddCommentRequestFinished(). Error: " + errorString());
    }
    
    disconnect(m_request, SIGNAL(finished()), this, SLOT(onAddCommentRequestFinished()));
    emit statusChanged(status());
}
