import { TextField } from '@material-ui/core';
import ToggleButton from '@material-ui/lab/ToggleButton';
import React, { useCallback, useEffect, useRef, useState, useMemo } from 'react';
import { PacketList } from '../PacketList/PacketList';
import { MessageFormat, WebSocketClient } from '../WebSocketClient/WebSocketClient';

const MAX_RENDER_PACKET_COUNT = 1000;
const RECEIVED_PACKET_QUEUE_CHECK_INTERVAL = 50;

export const MainPage = () => {
  const socketRef = useRef<WebSocket>(null);
  const [packets, setPackets] = useState<MessageFormat[]>([]);
  const [queue, _] = useState<MessageFormat[]>([]);
  const [isCapturing, setCapturing] = useState<boolean>(true);
  const [captureButtonText, setCaptureButtonText] = useState<string>('pause');
  const [captureFilter, setCaptureFilter] = useState<string>();
  const [displayFilter, setDisplayFilter] = useState<string>();
  const [clientDisplayFilter, setClientDisplayFilter] = useState<string>();

  // 受信済み(UI反映待ち)キューの監視
  useEffect(() => {
    const handle = setInterval(() => {
      if (queue.length > 0) {
        const item = queue.shift();
        setPackets(oldPackets => {
          // console.log(oldPackets.length);
          if (oldPackets.length === MAX_RENDER_PACKET_COUNT) {
            oldPackets.shift();
          }
          return oldPackets.concat(item);
        });
      }
    }, RECEIVED_PACKET_QUEUE_CHECK_INTERVAL);
    return () => clearInterval(handle);
  }, []);

  // キャプチャ状態変更時
  // 状態を変更しつつ、サーバへコマンド送信
  const onCaptureStateChanged = useCallback(() => {
    setCapturing(!isCapturing);
    if (isCapturing) {
      socketRef.current?.send('pause');
      setCapturing(false);
      setCaptureButtonText('resume');
    } else {
      socketRef.current?.send('resume');
      setCapturing(true);
      setCaptureButtonText('pause');
    }
  }, [isCapturing]);

  // 受信時
  const onMessageReceived = useCallback((msg: MessageFormat) => {
    // 非キャプチャ時は、持ってても仕方ないからキューをクリア
    if (isCapturing) {
      queue.push(msg);
    } else {
      queue.length = 0;
    }
  }, [isCapturing]);

  // フォーカス外れたら反映
  const onCaptureFilterChange = useCallback((e: React.FocusEvent<HTMLInputElement>) => {
    setCaptureFilter(e.currentTarget?.value ?? '');
  }, []);

  // フォーカス外れたら反映
  const onDisplayFilterChange = useCallback((e: React.FocusEvent<HTMLInputElement>) => {
    setDisplayFilter(e.currentTarget?.value ?? '');
  }, []);

  // リアルタイム反映
  const onClientDisplayFilterChange = useCallback((e: React.ChangeEvent<HTMLInputElement>) => {
    setClientDisplayFilter(e.currentTarget?.value ?? '');
  }, []);

  return (
    <div className='MainPage'>
      <div className='MainPage-ControllerContainer'>
        <div className='MainPage-CaptureButton'>
          <ToggleButton selected={isCapturing} onChange={onCaptureStateChanged}>{captureButtonText}</ToggleButton>
        </div>
        <div className='MainPage-CaptureFilter'>
          <TextField label='tshark capture filter' defaultValue={captureFilter} onBlur={onCaptureFilterChange} />
        </div>
        <div className='MainPage-DisplayFilter'>
          <TextField label='tshark display filter' defaultValue={displayFilter} onBlur={onDisplayFilterChange} />
        </div>
        <div className='MainPage-ClientDisplayFilter'>
          <TextField label='client display filter' defaultValue={clientDisplayFilter} onChange={onClientDisplayFilterChange} />
        </div>
      </div>
      <PacketList className='MainPage-PacketList' packets={packets} filter={clientDisplayFilter} />
      <WebSocketClient url={'ws://127.0.0.1:8080'} socketRef={socketRef} onMessageReceived={onMessageReceived} />
    </div>
  );
};
