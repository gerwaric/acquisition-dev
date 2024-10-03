#include <acquisition/endpoint_manager.h>

#include <acquisition/rate_limit/rate_limiter.h>

#include <json_struct/json_struct.h>
#include <QsLog/QsLog.h>

#include <QNetworkRequest>

namespace {

    struct LeagueListPayload {
        std::shared_ptr<std::vector<poe_api::League>> leagues;
        JS_OBJ(leagues);
    };

    struct CharacterListPayload {
        std::shared_ptr<std::vector<poe_api::Character>> characters;
        JS_OBJ(characters);
    };

    struct CharacterPayload {
        std::shared_ptr<poe_api::Character> character;
        JS_OBJ(character);
    };

    struct StashListPayload {
        std::shared_ptr<std::vector<poe_api::StashTab>> stashes;
        JS_OBJ(stashes);
    };

    struct StashPayload {
        std::shared_ptr<poe_api::StashTab> stash;
        JS_OBJ(stash);
    };

}

EndpointManager::EndpointManager(
    QObject* parent,
    RateLimiter& rate_limiter)
    : QObject(parent)
    , m_rate_limiter(rate_limiter) {}

void EndpointManager::getLeagues()
{
    const QUrl url("https://api.pathofexile.com/account/leagues");
    const QNetworkRequest request(url);
    auto* reply = m_rate_limiter.Submit("GET_LEAGUES", request);
    connect(reply, &RateLimitedReply::finished, this, &EndpointManager::receiveLeagueList);
}

void EndpointManager::listCharacters()
{
    const QUrl url("https://api.pathofexile.com/character");
    const QNetworkRequest request(url);
    auto* reply = m_rate_limiter.Submit("LIST_CHARACTERS", request);
    connect(reply, &RateLimitedReply::finished, this, &EndpointManager::receiveCharacterList);
}

void EndpointManager::getCharacter(
    const QString& name)
{
    const QUrl url("https://api.pathofexile.com/character/" + name);
    const QNetworkRequest request(url);
    auto* reply = m_rate_limiter.Submit("GET_CHARACTER", request);
    connect(reply, &RateLimitedReply::finished, this, &EndpointManager::receiveCharacter);
}

void EndpointManager::listStashes(
    const QString& league)
{
    const QUrl url("https://api.pathofexile.com/stash/" + league);
    const QNetworkRequest request(url);
    auto* reply = m_rate_limiter.Submit("LIST_STASHES", request);
    connect(reply, &RateLimitedReply::finished, this, &EndpointManager::receiveStashList);
}

void EndpointManager::getStash(
    const QString& league,
    const QString& stash_id,
    const QString& substash_id)
{
    const QString stash_location = stash_id + ((substash_id.isEmpty()) ? "" : "/" + substash_id);
    const QUrl url("https://api.pathofexile.com/stash/" + league + "/" + stash_location);
    const QNetworkRequest request(url);
    auto* reply = m_rate_limiter.Submit("GET_STASH", request);
    connect(reply, &RateLimitedReply::finished, this, &EndpointManager::receiveStash);
}

void EndpointManager::receiveLeagueList(QNetworkReply* reply)
{
    LeagueListPayload payload;
    const QByteArray bytes = reply->readAll();
    JS::ParseContext parseContext(bytes);
    if (parseContext.parseTo(payload) != JS::Error::NoError) {
        QLOG_ERROR() << "Error parsing character list payload:" << parseContext.makeErrorString();
        payload.leagues = {};
    };
    emit leagueListReceived(payload.leagues);
}

void EndpointManager::receiveCharacterList(QNetworkReply* reply)
{
    CharacterListPayload payload;
    const QByteArray bytes = reply->readAll();
    JS::ParseContext parseContext(bytes);
    if (parseContext.parseTo(payload) != JS::Error::NoError) {
        QLOG_ERROR() << "Error parsing character list payload:" << parseContext.makeErrorString();
        payload.characters = {};
    };
    emit characterListReceived(payload.characters);
}

void EndpointManager::receiveCharacter(QNetworkReply* reply)
{
    CharacterPayload payload;
    const QByteArray bytes = reply->readAll();
    JS::ParseContext parseContext(bytes);
    if (parseContext.parseTo(payload) != JS::Error::NoError) {
        QLOG_ERROR() << "Error parsing character payload:" << parseContext.makeErrorString();
        payload.character.reset();
    };
    if (payload.character) {
        emit characterReceived(payload.character);
    };
}

void EndpointManager::receiveStashList(QNetworkReply* reply)
{
    StashListPayload payload;
    const QByteArray bytes = reply->readAll();
    JS::ParseContext parseContext(bytes);
    if (parseContext.parseTo(payload) != JS::Error::NoError) {
        QLOG_ERROR() << "Error parsing stash list payload:" << parseContext.makeErrorString();
        payload.stashes = {};
    };
    emit stashListReceived(payload.stashes);
}

void EndpointManager::receiveStash(QNetworkReply* reply)
{
    StashPayload payload;
    const QByteArray bytes = reply->readAll();
    JS::ParseContext parseContext(bytes);
    if (parseContext.parseTo(payload) != JS::Error::NoError) {
        QLOG_ERROR() << "Error parsing stash payload:" << parseContext.makeErrorString();
        payload.stash.reset();
    };
    if (payload.stash) {
        emit stashReceived(payload.stash);
    };
}
