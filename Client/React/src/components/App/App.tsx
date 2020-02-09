import React, { useCallback, useEffect, useState } from 'react';
import { PacketList } from '../PacketList/PacketList';

const sock = new WebSocket('ws://127.0.0.1:8080');

export type MessageFormat = {
  timestamp: string,
  layers: {
    ip_src: string[],
    tcp_srcport: string[],
  }
};

export const App = () => {
  const [items, setItems] = useState<MessageFormat[]>([]);

  useEffect(() => {
    sock.addEventListener('open', e => {
      console.log('ws opened');
    });
    sock.addEventListener('message', e => {
      onMessageReceived(e);
    });
  }, []);

  const onMessageReceived = useCallback((e: MessageEvent) => {
    console.log(e.data);
    const obj = JSON.parse(e.data);
    const keys = Object.keys(obj);
    if (!keys.includes('layers')) {
      return;
    }
    const newItem = obj as MessageFormat;
    setItems(oldItems => [...oldItems, newItem]);
  }, [items]);

  const onPauseClick = useCallback(() => {
    sock.send('pause');
  }, []);

  const onResumeClick = useCallback(() => {
    sock.send('resume');
  }, []);

  return (
    <div className='App'>
      <button onClick={onPauseClick}>pause</button>
      <button onClick={onResumeClick}>resume</button>
      <PacketList packets={items} />
    </div>
  );
};
