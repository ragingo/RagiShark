import { useRef, useState, useEffect, useCallback } from 'react';

export type MessageFormat = {
  timestamp: string,
  layers: {
    frame_number: string[],
    ip_src: string[],
    ip_dst: string[],
    tcp_srcport: string[],
    tcp_dstport: string[],
    ip_proto: string[],
  }
};

type Props = {
  url: string,
  socketRef: React.MutableRefObject<WebSocket>,
  onMessageReceived: (MessageFormat) => void
};

export const WebSocketClient = (
    { url, socketRef, onMessageReceived }: Props
) => {
  useEffect(() => {
    if (socketRef.current) {
      return;
    }
    const sock = new WebSocket(url);
    sock.onopen = e => {
      console.log('ws opened');
    };
    sock.onmessage = e => {
      _onMessageReceived(e);
    };
    socketRef.current = sock;
  });

  const _onMessageReceived = useCallback((e: MessageEvent) => {
    console.log(e.data);
    const obj = JSON.parse(e.data);
    const keys = Object.keys(obj);
    if (!keys.includes('layers')) {
      return;
    }
    const newItem = obj as MessageFormat;
    onMessageReceived(newItem);
  }, []);

  return null;
};
