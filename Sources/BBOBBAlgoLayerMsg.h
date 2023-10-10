//
// Created by lardeur on 09/10/23.
//

#ifndef FBAE_BBOBBALGOLAYERMSG_H
#define FBAE_BBOBBALGOLAYERMSG_H

namespace fbae_BBOBBAlgoLayer {


    enum class BBOBBMsgId : unsigned char {
        Message,
    };

    struct Message {
        BBOBBMsgId msgId;


    };

}

#endif //FBAE_BBOBBALGOLAYERMSG_H
