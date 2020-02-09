import { useRef, useState, useEffect, useCallback } from 'react';

export type MessageFormat = {
  timestamp: string,
  layers: {
    ip_src: string[],
    tcp_srcport: string[],
  }
};

type Props = {
  url: string,
  socketRef: React.Ref<WebSocket>,
  onMessageReceived: (MessageFormat) => void
};

export const WebSocketClient = (
    { url, socketRef, onMessageReceived }: Props
) => {
  const ref = useRef<WebSocket>(null);
  socketRef = ref;

  useEffect(() => {
    if (ref.current) {
      return;
    }
    const sock = new WebSocket(url);
    sock.onopen = e => {
      console.log('ws opened');
    };
    sock.onmessage = e => {
      _onMessageReceived(e);
    };
    ref.current = sock;
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
