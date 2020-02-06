import React from 'react';

type Props = {
  packets: string[]
};

export const PacketList = ({ packets }: Props) => {
  return (
    <table className='PacketList'>
      <thead className='PacketListHeader'>
        <tr>
          <td>パケット一覧</td>
        </tr>
      </thead>
      <tbody>
        {packets.map((item, i) => (
          <tr key={i} className='PacketListItem'>
            <td key={i}>{item}</td>
          </tr>
        ))}
      </tbody>
    </table>
  );
};
