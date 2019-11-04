#include <stdio.h>
#include <vector>
#include <opencv2/opencv.hpp>
#include <tgbot/tgbot.h>
#include "ArtRobot/ArtRobot.h"
#include "ArtRobot/ArtRobot.h"

using namespace std;
using namespace cv;
using namespace TgBot;
using namespace ArtRobot;

int main()
{
    cout << "ThrowItBot start!" << endl;

    string token = getenv("TOKEN");
    Bot bot(token);

    bot.getEvents().onAnyMessage([&bot](Message::Ptr message) {
        printf("User wrote %s\n", message->text.c_str());
        if (
            StringTools::startsWith(message->text, "/start") ||
            StringTools::startsWith(message->text, "/help") ||
            StringTools::startsWith(message->text, "/throw") ||
            false)
        {
            return;
        }
        bot.getApi().sendMessage(message->chat->id, "Do you need /help ?", false, 0, std::make_shared<GenericReply>(), "", true);
    });

    bot.getEvents().onCommand("help", [&bot](Message::Ptr message) {
        bot.getApi().sendMessage(message->chat->id, "You can say /throw to throw youself!\nAnd you can forward someone's message for me to throw he.", false, 0, std::make_shared<GenericReply>(), "", true);
    });

    bot.getEvents().onCommand("start", [&bot](Message::Ptr message) {
        bot.getApi().sendMessage(message->chat->id, "Do you need to be /throw ?", false, 0, std::make_shared<GenericReply>(), "", true);
    });

    bot.getEvents().onCommand("throw", [&bot](Message::Ptr message) {
        bot.getApi().sendChatAction(message->chat->id, "upload_photo");

        auto photos = bot.getApi().getUserProfilePhotos(message->chat->id);
        auto userImgPath = bot.getApi().getFile(photos->photos[0][1]->fileId);
        auto userImgData = bot.getApi().downloadFile(userImgPath->filePath); // 图像数据（maybe jpg）

        auto body = Component::Group();                         // body
        auto bg = Component::Image(0, 0, 512, 512, 0, "p.png"); // bg
        body.addChild(bg.getSurface());                         // Show bg

        vector<unsigned char> userImgVector(userImgData.begin(), userImgData.end()); // 用户头像
        Mat userImgMat = imdecode(userImgVector, IMREAD_COLOR);
        auto userImg = Component::Image(18.56,
                                        180.98,
                                        135.53,
                                        135.53,
                                        -160,
                                        userImgMat);

        auto mask = Component::ImageMask(0, 0, 512, 512, 0, "p_mask.png", userImg.getSurface()); // Mask
        body.addChild(mask.getSurface());                                                        // Show mask

        // Renderer renderer(OutputTypePixmap, 512, 512, Renderer::PX, 72);
        Renderer renderer(OutputTypePng, 512, 512, Renderer::PX, 72);
        renderer.render(body.getSurface());

        auto fileNew = make_shared<InputFile>();
        fileNew->data = renderer.getDataString();
        fileNew->mimeType = "image/png";
        // bot.getApi().sendPhoto(message->chat->id, fileNew, "", 0, std::make_shared<GenericReply>(), "", true);
        bot.getApi().sendSticker(message->chat->id, fileNew, 0, std::make_shared<GenericReply>(), true);
        bot.getApi().sendMessage(message->chat->id, "( ﹁ ﹁ ) ", false, 0, std::make_shared<GenericReply>(), "", true);
    });

    while (true)
    {
        try
        {
            cout << "Connecting to telegram API server" << endl;
            cout << "Bot username: " << bot.getApi().getMe()->username << endl;
            TgLongPoll longPoll(bot);
            while (true)
            {
                printf("Long poll started\n");
                longPoll.start();
            }
        }
        catch (TgException &e)
        {
            cerr << "error: " << e.what() << endl;
        }
    }

    return 0;
}