import React from 'react';
import { PacketMessageFormat } from '../WebSocketClient/WebSocketClient';

type Props = {
  packets: PacketMessageFormat[],
  filter?: string,
  className?: string
};

export const PacketList = ({ packets, filter, className }: Props) => {
  return (
    <div className={className}>
      <table className='PacketList'>
        <thead className='PacketListHeader'>
          <tr>
            {createColumnHeaders().map((h, i) => (
              <td key={i}>{h.label}</td>
            ))}
          </tr>
        </thead>
        <tbody>
          {packets.filter(x => createPacketFilter(x, filter))
            .map(createColumns)
            .map((cols, rowIdx) => (
              <tr key={rowIdx} className='PacketListItem'>
                {cols.map((col, colIdx) => (
                  <td key={colIdx}>{col.value}</td>
                ))}
              </tr>
          ))}
        </tbody>
      </table>
    </div>
  );
};

const createPacketFilter = (msg: PacketMessageFormat, exp: string) => {
  if (!exp || exp.length === 0) {
    return true;
  }
  try {
    return !!JSON.stringify(msg).match(exp);
  } catch {
    return false;
  }
};

const createColumnHeaders = () => {
  const columns = [
    { label: 'タイムスタンプ' },
    { label: 'No' },
    { label: '送信元IP' },
    { label: '送信元ポート' },
    { label: '送信先IP' },
    { label: '送信先ポート' },
    { label: 'プロトコルNo' },
  ];
  return columns;
};

const createColumns = (msg: PacketMessageFormat) => {
  const columns = [
    { value: msg.timestamp },
    { value: msg.layers.frame_number },
    { value: msg.layers.ip_src },
    { value: msg.layers.tcp_srcport },
    { value: msg.layers.ip_dst },
    { value: msg.layers.tcp_dstport },
    { value: msg.layers.ip_proto },
  ];
  return columns;
};
