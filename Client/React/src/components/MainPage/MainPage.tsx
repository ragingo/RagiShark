import { Button, TextField } from '@material-ui/core';
import ToggleButton from '@material-ui/lab/ToggleButton';
import React, { useCallback, useEffect, useRef, useState } from 'react';
import { PacketList } from '../PacketList/PacketList';
import { MessageFormat, PacketMessageFormat, WebSocketClient, GetIFListCommandMessageFormat } from '../WebSocketClient/WebSocketClient';

const MAX_RENDER_PACKET_COUNT = 500;
const RECEIVED_PACKET_QUEUE_CHECK_INTERVAL = 50;

export const MainPage = () => {
  const socketRef = useRef<WebSocket>(null);
  const [interfaces, setInterfaces] = useState<{ no: number, name: string}[]>([]);
  const [packets, setPackets] = useState<PacketMessageFormat[]>([]);
  const [queue, _] = useState<PacketMessageFormat[]>([]);
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
            oldPackets.pop();
          }
          return [item, ...oldPackets];
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
      queue.length = 0;
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
    if (msg as GetIFListCommandMessageFormat) {
      const iflist = msg as GetIFListCommandMessageFormat;
      setInterfaces([...iflist.data]);
      return;
    }
    // 非キャプチャ時は、持ってても仕方ないからキューをクリア
    if (isCapturing) {
      queue.push(msg as PacketMessageFormat);
    } else {
      queue.length = 0;
    }
  }, [isCapturing]);

  // フォーカス外れたら反映
  const onCaptureFilterChange = useCallback((e: React.FocusEvent<HTMLInputElement>) => {
    const value = e.currentTarget?.value ?? '';
    setCaptureFilter(value);
    sendCommand(socketRef, 'change cf', value);
  }, []);

  // フォーカス外れたら反映
  const onDisplayFilterChange = useCallback((e: React.FocusEvent<HTMLInputElement>) => {
    const value = e.currentTarget?.value ?? '';
    setDisplayFilter(value);
    sendCommand(socketRef, 'change df', value);
  }, []);

  // リアルタイム反映
  const onClientDisplayFilterChange = useCallback((e: React.ChangeEvent<HTMLInputElement>) => {
    const value = e.currentTarget?.value ?? '';
    setClientDisplayFilter(value);
  }, []);

  const onClearButtonClick = useCallback(() => {
    setPackets([]);
    queue.length = 0;
  }, []);

  const onGetIFListButtonClick = useCallback(() => {
    sendCommand(socketRef, 'get if list');
  }, []);

  return (
    <div className='MainPage'>
      <div className='MainPage-ControllerContainer'>
        <div className='MainPage-GetIFListButton'>
          <Button onClick={onGetIFListButtonClick} variant='contained'>load</Button>
        </div>
        <div className='MainPage-GetIFList'>
          <select>
            {interfaces.map(x => (
              <option key={x.no}>{x.no} : {x.name}</option>
            ))}
          </select>
        </div>
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
        <div className='MainPage-ClearButton'>
          <Button onClick={onClearButtonClick} variant='contained'>Clear</Button>
        </div>
      </div>
      <PacketList className='MainPage-PacketList' packets={packets} filter={clientDisplayFilter} />
      <WebSocketClient url={'ws://127.0.0.1:8080'} socketRef={socketRef} onMessageReceived={onMessageReceived} />
    </div>
  );
};

type WebSocketCommands = 'change cf' | 'change df' | 'get if list';

const sendCommand = (sock: React.RefObject<WebSocket>, cmd: WebSocketCommands, value?: string) => {
  sock.current?.send(`${cmd} ${value ?? ''}`.trim());
};
