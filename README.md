# GMEXT-Steamworks но с блинами и икрой...

# RU:

# РЕЛИЗ ДЛЯ VK PLAY [ЗДЕСЬ](https://github.com/nkrapivin/GMEXT-Steamworks/releases/tag/v1.1.4vk)

Репозиторий для официального расширения Steamworks от YoYoGames Ltd. (авторов GameMaker), но с поддержкой VK Play.

Работает только на `GameMaker 2022.9` (он же `GameMaker-LTS 2022.0`) или новее.

Требует Steamworks SDK v1.50 и VK Play Client DLL `06.06.99.59/Release (ну или Debug)/Steam_API64.dll` из архива.

Отличие `Debug` дллки от `Release` в том, что она пишет Fume.log (ого, кодовое имя эмулятора - Fume! мило!)

Эти логи можно отправлять в `Integration Team` в случае проблем, так что `Debug` версия иногда полезна.

Первое можно скачать вот [здеся](https://partner.steamgames.com/downloads/steamworks_sdk_150.zip).

Второе вот [здеся](https://admin.dl.mail.ru/guptools/steam_api_dlls.zip).

Советую сделать отдельную папку с SDK Стима но для VK Play билдов и заменить файл `sdk\redistributable_bin\win64\steam_api64.dll` на файл из архива VK Play.

Ваш AppID указан в `Системные свойства` в кабинете разработчика VK Play, если он не указан, пожалуйста, свяжитесь с `Integration Team` в чате, они включат, они добрые :)

Собирать под любые платформы кроме Windows пока бесполезно, т.к. у Фумы нет порта ни под Лялих ни под Макось.

## Для старых GameMaker: Studio и Game Maker (те что на Дельфи были написаны, бррр)

### GameMaker: Studio 1.4

Вам это расширение не нужно, так как поддержка Стима была встроенна, НО!

У вас уже должен быть где-то распакован SDK `v1.35a` который нужен для Студии 1.4, потом возьмите дллку из архива VK Play версии `03.27.76.74`.

### GameMaker Studio 2.2.xxx - 2022.2.xxx

Вам это расширение не нужно, так как поддержка Стима была встроенна, НО!

У вас уже должен быть где-то распакован SDK `v1.42` который нужен для этих версий, потом возьмите дллку из архива VK Play версии `04.28.51.07`.

### GameMaker >= 2022.3+

Вам нужно это расширение, т.к. версии от YoYoGames используют слишком новый SDK `v1.53a` или новее, а последний у VK Play только `v1.50`.

### GameMaker <= 8.1.xxx

Вы с ума посходили? Совсем уже? Этой версии двигла чуть ли не 12 лет, обновитесь!

# EN:

Repository for the official Steamworks extension by YoYoGames Ltd. (GameMaker's authors), but with VK Play support.

Works only on `GameMaker 2022.9` (also called `GameMaker-LTS 2022.0`) or newer.

Requires Steamworks SDK v1.50 and the VK Play Client DLL `06.06.99.59/Release (or Debug)/Steam_API64.dll` из архива.

The difference between `Debug` and `Release` is in logging, the debug version writes a Fume.log that logs all virtual table calls.

These logs can be sent to the `Integration Team` in your dev chat if you have issues with the integration, so `Debug`'s not completely useless!

The first thing can be downloaded [here](https://partner.steamgames.com/downloads/steamworks_sdk_150.zip).

The second one is [here](https://admin.dl.mail.ru/guptools/steam_api_dlls.zip).

I recommend making a new folder with the Steamworks SDK just for VK Play builds and then replace `sdk\redistributable_bin\win64\steam_api64.dll` with VK Play's from the archive.

Your AppID is specified in the `System attributes` panel in your VK Play Developer Dashboard, if it is not there, please, contact `Integration Team` in your chat, they're kind folks :)

Building for any platforms other than Windows is silly for now, since Fume has no ports for Linux and macOS.
