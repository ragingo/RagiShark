import { Button, Checkbox, Input, ListItemText, MenuItem, Select, TextField } from '@material-ui/core';
import ToggleButton from '@material-ui/lab/ToggleButton';
import React, { useCallback, useEffect, useRef, useState } from 'react';
import { PacketList } from '../PacketList/PacketList';
import { isGetIFListCommandMessage, isPacketMessage, MessageFormat, PacketMessageFormat, WebSocketClient } from '../WebSocketClient/WebSocketClient';

const MAX_RENDER_PACKET_COUNT = 500;
const RECEIVED_PACKET_QUEUE_CHECK_INTERVAL = 50;
const WS_SERVER_URL = process.env.WS_SERVER_URL || '';

type NicInfo = {
  no: number;
  name: string;
  checked: boolean;
};

export const MainPage = () => {
  const socketRef = useRef<WebSocket>(null);
  const [interfaces, setInterfaces] = useState<NicInfo[]>([]);
  const [packets, setPackets] = useState<PacketMessageFormat[]>([]);
  const [queue, _] = useState<PacketMessageFormat[]>([]);
  const [isStarted, setStarted] = useState<boolean>(false);
  const [isCapturing, setCapturing] = useState<boolean>(false);
  const [captureButtonText, setCaptureButtonText] = useState<string>('start');
  const [captureFilter, setCaptureFilter] = useState<string>('');
  const [displayFilter, setDisplayFilter] = useState<string>('');
  const [clientDisplayFilter, setClientDisplayFilter] = useState<string>('');

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
    if (isStarted) {
      setCapturing(!isCapturing);
      if (isCapturing) {
        queue.length = 0;
        setCapturing(false);
        setCaptureButtonText('resume');
        sendCommand(socketRef, 'pause');
      } else {
        setCapturing(true);
        setCaptureButtonText('pause');
        sendCommand(socketRef, 'resume');
      }
    } else {
      setStarted(true);
      setCapturing(true);
      setCaptureButtonText('pause');
      sendCommand(socketRef, 'start', `-cf ${captureFilter} -i ${interfaces.filter(x => x.checked).map(x => x.no)}`);
    }
  }, [isStarted, isCapturing, captureFilter, interfaces]);

  // 受信時
  const onMessageReceived = useCallback((msg: MessageFormat) => {
    if (isGetIFListCommandMessage(msg)) {
      const empty = { no: 0, name: '', checked: false } as NicInfo;
      const ifs = msg.data.map(x => { return { no: x.no, name: x.name, checked: false } as NicInfo; });
      setInterfaces([empty, ...ifs]);
      return;
    }
    if (isPacketMessage(msg)) {
      queue.push(msg);
    }
  }, []);

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

  const onIFListSelectionChanged = useCallback((e: React.ChangeEvent<{ value: NicInfo[] }>) => {
    // 変更されてないやつしか含まれていない・・・謎
    const unchangedItems = e.target.value;
    interfaces.forEach(x => {
      if (!unchangedItems.map(x => x.no).includes(x.no)) {
        x.checked = !x.checked;
      }
    });
    setInterfaces([...interfaces]);

    if (isStarted) {
      const selectedIFs = interfaces.filter(x => x.checked).map(x => x.no);
      console.log(`selected interfaces: ${JSON.stringify(selectedIFs)}`);
      sendCommand(socketRef, `set if list`, selectedIFs.join(','));
    }
  }, [interfaces, isStarted]);

  return (
    <div className='MainPage'>
      <div className='MainPage-ControllerContainer'>
        <div className='MainPage-GetIFListButton'>
          <Button onClick={onGetIFListButtonClick} variant='contained'>load</Button>
        </div>
        <div className='MainPage-GetIFList'>
          <Select onChange={onIFListSelectionChanged} multiple input={<Input />} value={interfaces} renderValue={showSelectedValues}>
            {interfaces.map(x => (
              <MenuItem key={x.no} value={x as any}>
                <Checkbox checked={x.checked} />
                <ListItemText primary={x.name} />
              </MenuItem>
            ))}
          </Select>
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
      <WebSocketClient url={WS_SERVER_URL} socketRef={socketRef} onMessageReceived={onMessageReceived} />
    </div>
  );
};

type WebSocketCommands = 'start' | 'pause' | 'resume' | 'change cf' | 'change df' | 'get if list' | 'set if list';

const sendCommand = (sock: React.RefObject<WebSocket>, cmd: WebSocketCommands, value?: string | number | string[] | number[]) => {
  if (Array.isArray(value)) {
    // TODO: あとで実装する
  } else {
    const msg = `${cmd} ${value ?? ''}`.trim();
    console.log(msg);
    sock.current?.send(msg);
  }
};

const showSelectedValues = (values: NicInfo[]) => {
  return values.filter(x => x.checked)
    .map(x => x.no)
    .join(', ');
};
