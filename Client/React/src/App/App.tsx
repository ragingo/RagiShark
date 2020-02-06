import React, { useCallback, useEffect, useState } from 'react';

const sock = new WebSocket('ws://127.0.0.1:8080');

export const App = () => {
  const [items, setItems] = useState<string[]>([]);

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
    const newItem = e.data as string;
    setItems(oldItems => [...oldItems, newItem]);
  }, [items]);

  const onClick = useCallback(() => {
    sock.send('test!');
  }, []);

  return (
    <div className='App'>
      <button onClick={onClick}>send</button>
      <table className='PacketList'>
        <thead className='PacketListHeader'>
          <tr>
            <td>パケット一覧</td>
          </tr>
        </thead>
        <tbody>
          {items.map((item, i) => (
            <tr className='PacketListItem'>
              <td key={i}>{item}</td>
            </tr>
          ))}
        </tbody>
      </table>
    </div>
  );
};
