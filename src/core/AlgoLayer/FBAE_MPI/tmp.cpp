auto task_to_receive_msg = std::async(std::launch::async, [this] {


  while (receive) {




    // Créer les messages à envoyer à ??? Session/Comm ???
    std::string s;
    {
      lock_guard lck(mtxBatchCtrl);
      s = {serializeStruct<Message>(Message{MsgId::Message,
                                            static_cast<rank_t>(rank),
                                            msgsWaitingToBeBroadcast})};
      msgsWaitingToBeBroadcast.clear(); // Clear tout les messages à envoyer
    }


    int msgSize = s.size(); //Size of message




      // size = Nomber of broadcasters

    // Procède à la mise en commun de la taille des messages de tous les processus
    // Gather the size of each message
    std::vector<int> message_sizes(size); // Create a vector of size size
    MPI_Allgather(&msgSize, 1, MPI_INT, message_sizes.data(), 1, MPI_INT, // Gathers data from all tasks and distribute the combined data to all tasks
                  MPI_COMM_WORLD);





    // Calculate total size of messages
    int total_message_size = 0;
    for (int i = 0; i < size; ++i) {
      total_message_size += message_sizes[i];
    }




    // Allocate buffer for receiving messages
    std::vector<char> buffer(total_message_size);





    // Calculate displacement array for allgatherv
    std::vector<int> displacements(size);
    displacements[0] = 0;
    for (int i = 1; i < size; ++i) {
      displacements[i] = displacements[i - 1] + message_sizes[i - 1]; // Ou commence message précédent + taille message prcécédent
    }




    // Une fois qu'on connait leur taille, c'est facile de créer un buffer de taille adéquat
    
    // Gather the messages
    MPI_Allgatherv(s.data(), msgSize, MPI_BYTE, buffer.data(),
                   message_sizes.data(), displacements.data(), MPI_BYTE,
                   MPI_COMM_WORLD);



    // C'est bon on a récupéré tous les messages






    int offset = 0;
    for (int i = 0; i < size; i++) {


      string msg(buffer.data() + offset,
                 buffer.data() + offset + message_sizes[i]);







      auto deserializedMsg{deserializeStruct<Message>(std::move(msg))};


      cout << deserializedMsg.senderRank;



      for (string h : deserializedMsg.batchesBroadcast) {
        // We surround the call to @callbackDeliver method with
        // shortcutBatchCtrl = true; and shortcutBatchCtrl = false; This is
        // because callbackDeliver() may lead to a call to
        // @totalOrderBroadcast method which could get stuck in
        // condVarBatchCtrl.wait() instruction because task
        // @SessionLayer::sendPeriodicPerfMessage may have filled up
        // @msgsWaitingToBeBroadcast
        shortcutBatchCtrl = true;
        getSession()->callbackDeliver(deserializedMsg.senderRank, std::move(h));
        shortcutBatchCtrl = false;
      }


      offset += message_sizes[i];
    }
  }




});


task_to_receive_msg.get();