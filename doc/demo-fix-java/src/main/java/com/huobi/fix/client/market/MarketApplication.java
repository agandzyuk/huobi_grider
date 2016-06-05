package com.huobi.fix.client.market;

import quickfix.Application;
import quickfix.DoNotSend;
import quickfix.FieldNotFound;
import quickfix.IncorrectDataFormat;
import quickfix.IncorrectTagValue;
import quickfix.Message;
import quickfix.MessageCracker;
import quickfix.RejectLogon;
import quickfix.Session;
import quickfix.SessionID;
import quickfix.SessionNotFound;
import quickfix.UnsupportedMessageType;

public class MarketApplication extends MessageCracker implements Application {
	private SessionID sessionID;

	public MarketApplication() {

	}

	@Override
	public void onCreate(SessionID sessionID) {
		this.sessionID = sessionID;
	}

	@Override
	public void onLogon(final SessionID sessionID) {
		new Thread(new Runnable() {
			@Override
			public void run() {
				try {
					quickfix.Message message = MarketClientApi.marketDataDepthRequest("BTC/CNY", 10, '1');
					Session.lookupSession(sessionID).send(message);
				} catch (SessionNotFound e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
				
			}
		}).start();
	}

	@Override
	public void onLogout(SessionID sessionID) {
        System.err.println("客户端登出: " + sessionID.toString());
	}

	@Override
	public void toAdmin(Message message, SessionID sessionID) {
    	System.err.println("客户端发送的管理消息：" + message.toString());
	}

	@Override
	public void fromAdmin(Message message, SessionID sessionID)
			throws FieldNotFound, IncorrectDataFormat, IncorrectTagValue,
			RejectLogon {
    	System.out.println("客户端收到的管理消息：" + message.toString());
	}

	@Override
	public void toApp(Message message, SessionID sessionID) throws DoNotSend {
    	System.err.println("客户端发送的应用消息：" + message.toString());
	}

	@Override
	public void fromApp(Message message, SessionID sessionID)
			throws FieldNotFound, IncorrectDataFormat, IncorrectTagValue,
			UnsupportedMessageType {
    	System.out.println("客户端收到的应用消息：" + message.toString());
//		crack(message, sessionID);
	}

	/*public void onMessage(final quickfix.fix44.MarketDataSnapshotFullRefresh message, SessionID sessionID) {
	}*/

	protected void onMessage(Message message, SessionID sessionID)
			throws FieldNotFound, UnsupportedMessageType, IncorrectTagValue {
		System.out.println("客户端接收到的信息：" + message.toString());
	}

	public boolean sendMessage(Message m) {
		if (sessionID != null) return Session.lookupSession(sessionID).send(m); else return false;
	}
}
