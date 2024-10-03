# acquisition-dev

A playground for testing new ideas

## Redesign

Project structure inspired by Boris Kolpackov's [Canonical Project Structure](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p1204r0.html)

#### Dependencies

[json_struct](https://github.com/jorgen/json_struct/) instead of [RapidJSON](https://rapidjson.org/) because `json_struct` makes it much simpler to deserialize json, and we don't need the performance of RapidJSON.

[QsLog](https://github.com/gerwaric/QsLog) forked from [victronenergy/QsLog](https://github.com/victronenergy/QsLog) so I could make minor changes such as adding `.gitignore`.

[boost-headers-only](https://github.com/gerwaric/boost-headers-only) forked from [scipy/boost-headers-only](https://github.com/scipy/boost-headers-only) so that I could more easily update the headers to newer versions of boost.