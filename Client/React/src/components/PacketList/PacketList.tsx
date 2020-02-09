import React from 'react';
import { MessageFormat } from '../WebSocketClient/WebSocketClient';

type Props = {
  packets: MessageFormat[]
};

export const PacketList = ({ packets }: Props) => {
  return (
    <table className='PacketList'>
      <thead className='PacketListHeader'>
        <tr>
          <td>タイムスタンプ</td>
          <td>送信元IP</td>
          <td>送信元ポート</td>
        </tr>
      </thead>
      <tbody>
        {packets.map((item, i) => (
          <tr key={i} className='PacketListItem'>
            <td key={0}>{item.timestamp}</td>
            <td key={1}>{item.layers.ip_src}</td>
            <td key={2}>{item.layers.tcp_srcport}</td>
          </tr>
        ))}
      </tbody>
    </table>
  );
};
