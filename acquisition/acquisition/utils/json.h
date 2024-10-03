#pragma once

#include <acquisition/utils/utils.h>

#include <json_struct/json_struct.h>

#include <QByteArray>
#include <QDateTime>
#include <QString>

namespace JS {

    template<>
    struct TypeHandler<QByteArray>
    {
    public:
        static inline Error to(QByteArray& to_type, JS::ParseContext& context)
        {
            const std::string value(
                context.token.value.data,
                context.token.value.size);
            to_type = QByteArray::fromStdString(value);
            return Error::NoError;
        }
        static void from(const QByteArray& from_type, JS::Token& token, JS::Serializer& serializer)
        {
            const std::string value = from_type.toStdString();
            token.value_type = JS::Type::String;
            token.value.data = value.data();
            token.value.size = value.size();
            serializer.write(token);
        }
    };

    template<>
    struct TypeHandler<QString>
    {
    public:
        static inline Error to(QString& to_type, JS::ParseContext& context)
        {
            const std::string value(
                context.token.value.data,
                context.token.value.size);
            to_type = QString::fromStdString(value);
            return Error::NoError;
        }
        static void from(const QString& from_type, JS::Token& token, JS::Serializer& serializer)
        {
            const std::string value = from_type.toStdString();
            token.value_type = JS::Type::String;
            token.value.data = value.data();
            token.value.size = value.size();
            serializer.write(token);
        }
    };

    template<>
    struct TypeHandler<QDateTime>
    {
    public:
        static inline Error to(QDateTime& to_type, JS::ParseContext& context)
        {
            const QByteArray value(
                context.token.value.data,
                context.token.value.size);
            to_type = QDateTime::fromString(utils::replace_obsolete_timezones(value), Qt::RFC2822Date);
            if (!to_type.isValid()) {
                return Error::UserDefinedErrors;
            };
            return Error::NoError;
        }
        static void from(const QDateTime& from_type, JS::Token& token, JS::Serializer& serializer)
        {
            const std::string buf = from_type.toString(Qt::RFC2822Date).toStdString();
            token.value_type = JS::Type::String;
            token.value.data = buf.data();
            token.value.size = buf.size();
            serializer.write(token);
        }
    };

    template <typename Value, typename Map>
    struct TypeHandlerMap<QString,Value,Map>
    {
        static inline Error to(Map& to_type, ParseContext& context)
        {
            if (context.token.value_type != Type::ObjectStart)
            {
                return JS::Error::ExpectedObjectStart;
            }

            Error error = context.nextToken();
            if (error != JS::Error::NoError)
                return error;
            while (context.token.value_type != Type::ObjectEnd)
            {
                std::string str;
                Internal::handle_json_escapes_in(context.token.name, str);
                QString key = QString::fromStdString(str);
                Value v;
                error = TypeHandler<Value>::to(v, context);
                to_type[std::move(key)] = std::move(v);
                if (error != JS::Error::NoError)
                    return error;
                error = context.nextToken();
            }

            return error;
        }

        static void from(const Map& from, Token& token, Serializer& serializer)
        {
            token.value_type = Type::ObjectStart;
            token.value = DataRef("{");
            serializer.write(token);
            for (auto it = from.begin(); it != from.end(); ++it)
            {
                token.name = DataRef(it->first.toStdString());
                token.name_type = Type::String;
                TypeHandler<Value>::from(it->second, token, serializer);
            }
            token.name.size = 0;
            token.name.data = "";
            token.name_type = Type::String;
            token.value_type = Type::ObjectEnd;
            token.value = DataRef("}");
            serializer.write(token);
        }
    };

}