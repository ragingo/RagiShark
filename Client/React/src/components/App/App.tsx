import React, { useCallback, useState, useRef } from 'react';
import { PacketList } from '../PacketList/PacketList';
import { WebSocketClient, MessageFormat } from '../WebSocketClient/WebSocketClient';

export const App = () => {
  const socketRef = useRef<WebSocket>(null);
  const [packets, setPackets] = useState<MessageFormat[]>([]);

  const onPauseClick = useCallback(() => {
    socketRef.current.send('pause');
  }, []);

  const onResumeClick = useCallback(() => {
    socketRef.current.send('resume');
  }, []);

  const onMessageReceived = useCallback((msg: MessageFormat) => {
    setPackets(oldPackets => oldPackets.concat(msg));
  }, []);

  return (
    <div className='App'>
      <button onClick={onPauseClick}>pause</button>
      <button onClick={onResumeClick}>resume</button>
      <PacketList packets={packets} />
      <WebSocketClient url={'ws://127.0.0.1:8080'} socketRef={socketRef} onMessageReceived={onMessageReceived} />
    </div>
  );
};
