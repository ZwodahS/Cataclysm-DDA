#include "messages.h"
#include <sstream>

// Messages object.
static Messages player_messages;

std::vector<Messages::game_message> Messages::recent_messages(const int count)
{
    int tmp_count = count;
    if (tmp_count > player_messages.messages.size()) {
        tmp_count = player_messages.messages.size();
    }
    return std::vector<game_message>(player_messages.messages.end() - tmp_count,
                                     player_messages.messages.end());
}; // AO:only used once, should be generalized and game_message become private

void Messages::add_msg_string(const std::string &s)
{
    if (s.length() == 0) {
        return;
    }
    if (!player_messages.messages.empty() &&
        int(player_messages.messages.back().turn) + 3 >= int(calendar::turn) &&
        s == player_messages.messages.back().message) {
        player_messages.messages.back().count++;
        player_messages.messages.back().turn = calendar::turn;
        return;
    }

    while (player_messages.messages.size() >= 256) {
        player_messages.messages.erase(player_messages.messages.begin());
    }

    player_messages.messages.push_back(game_message(calendar::turn, s));
}

void Messages::vadd_msg(const char *msg, va_list ap)
{
    player_messages.add_msg_string(vstring_format(msg, ap));
}

void Messages::add_msg(const char *msg, ...)
{
    va_list ap;
    va_start(ap, msg);
    vadd_msg(msg, ap);
    va_end(ap);
}

void Messages::clear_messages()
{
    player_messages.messages.clear();
}

size_t Messages::size()
{
    return player_messages.messages.size();
}

bool Messages::has_undisplayed_messages()
{
    return size() && (player_messages.messages.back().turn > player_messages.curmes);
}

void Messages::display_messages(WINDOW *ipk_target, int left, int top, int right, int bottom,
                                int offset, bool display_turns)
{

    //from  game
    if (!player_messages.messages.size()) {
        return;
    }
    int line = bottom;
    int maxlength = right - left;
    if (offset < 0) {
        offset = 0;
    }
    if (offset >= player_messages.messages.size()) {
        offset = player_messages.messages.size();
    }

    int lasttime = -1;
    for (int i = player_messages.messages.size() - offset - 1; i >= 0 && line >= top; i--) {
        game_message &m = player_messages.messages[i];
        std::string mstr = m.message;
        if (m.count > 1) {
            std::stringstream mesSS;
            //~ Message %s on the message log was repeated %d times, eg. �You hear a whack! x 12�
            mesSS << string_format(_("%s x %d"), mstr.c_str(), m.count);
            mstr = mesSS.str();
        }
        // Split the message into many if we must!
        nc_color col = c_dkgray;
        if (int(m.turn) >= player_messages.curmes) {
            col = c_ltred;
        } else if (int(m.turn) + 5 >= player_messages.curmes) {
            col = c_ltgray;
        }
        std::vector<std::string> folded = foldstring(mstr, maxlength);
        for (int j = folded.size() - 1; j >= 0 && line >= top; j--, line--) {
            mvwprintz(ipk_target, line, left, col, "%s", folded[j].c_str());
        }
        if (display_turns) {
            calendar timepassed = calendar::turn - m.turn;

            if (int(timepassed) > lasttime) {
                mvwprintz(ipk_target, line, 3, c_ltblue, _("%s ago:"),
                          timepassed.textify_period().c_str());
                line--;
                lasttime = int(timepassed);
            }
        }
    }
    player_messages.curmes = int(calendar::turn);
};
