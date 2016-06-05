package com.huobi.fix.client.market;

import java.util.UUID;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import quickfix.Initiator;
import quickfix.SessionNotFound;
import quickfix.field.MDEntryType;
import quickfix.field.MDReqID;
import quickfix.field.MDUpdateType;
import quickfix.field.MarketDepth;
import quickfix.field.SubscriptionRequestType;
import quickfix.field.Symbol;
import quickfix.field.NoRelatedSym;
import quickfix.fix44.MarketDataRequest;
import quickfix.fix44.component.Instrument;


public class MarketClientApi
{
	// 交易
	public static MarketDataRequest marketDataTradeRequest(String symbol, char subscribe) throws SessionNotFound
	{
		MarketDataRequest message = new MarketDataRequest();

		message.set(new NoRelatedSym(2));
		message.set(new MDReqID(symbol + UUID.randomUUID().toString()));
		message.set(new SubscriptionRequestType(subscribe));
		message.set(new MarketDepth(10)); 
		message.set(new MDUpdateType(MDUpdateType.FULL_REFRESH));

		MarketDataRequest.NoMDEntryTypes noMDEntryTypes = new MarketDataRequest.NoMDEntryTypes();
		noMDEntryTypes.set(new MDEntryType(MDEntryType.TRADE));
		message.addGroup(noMDEntryTypes);

		MarketDataRequest.NoRelatedSym noRelatedSym = new MarketDataRequest.NoRelatedSym();
		Instrument instrument = new Instrument();
		instrument.set(new Symbol(symbol));
		noRelatedSym.set(instrument);
		message.addGroup(noRelatedSym);
		
		return message;
	}
	
	// 深度
	public static MarketDataRequest marketDataDepthRequest(String symbol, int depth, char subscribe) throws SessionNotFound
	{
		MarketDataRequest message = new MarketDataRequest();

		message.set(new NoRelatedSym(2));
		message.set(new MDReqID(symbol + UUID.randomUUID().toString()));
		message.set(new SubscriptionRequestType(subscribe));
		message.set(new MarketDepth(depth));
		message.set(new MDUpdateType(MDUpdateType.FULL_REFRESH));

		MarketDataRequest.NoMDEntryTypes noMDEntryTypes = new MarketDataRequest.NoMDEntryTypes();
		noMDEntryTypes.set(new MDEntryType(MDEntryType.BID));
		message.addGroup(noMDEntryTypes);
		noMDEntryTypes.set(new MDEntryType(MDEntryType.OFFER));
		message.addGroup(noMDEntryTypes);

		/*MarketDataRequest.NoRelatedSym noRelatedSym = new MarketDataRequest.NoRelatedSym();
		Instrument instrument = new Instrument();
		instrument.set(new Symbol(symbol));
		noRelatedSym.set(instrument);
		message.addGroup(noRelatedSym);*/
		
		MarketDataRequest.NoRelatedSym noRelatedSym = new MarketDataRequest.NoRelatedSym();
		noRelatedSym.set(new Symbol("BTC/CNY"));
		message.addGroup(noRelatedSym);
		
		return message;
	}

	// 盘口
	public static MarketDataRequest marketDataTickerRequest(String symbol, char subscribe) throws SessionNotFound
	{
		MarketDataRequest message = new MarketDataRequest();

		message.set(new NoRelatedSym(2));
		message.set(new MDReqID(symbol + UUID.randomUUID().toString()));
		message.set(new SubscriptionRequestType(subscribe));
		message.set(new MarketDepth(10));
		message.set(new MDUpdateType(MDUpdateType.FULL_REFRESH));

		MarketDataRequest.NoMDEntryTypes noMDEntryTypes = new MarketDataRequest.NoMDEntryTypes();
		noMDEntryTypes.set(new MDEntryType(MDEntryType.OPENING_PRICE));
		message.addGroup(noMDEntryTypes);
		noMDEntryTypes.set(new MDEntryType(MDEntryType.CLOSING_PRICE));
		message.addGroup(noMDEntryTypes);
		noMDEntryTypes.set(new MDEntryType(MDEntryType.TRADING_SESSION_HIGH_PRICE));
		message.addGroup(noMDEntryTypes);
		noMDEntryTypes.set(new MDEntryType(MDEntryType.TRADING_SESSION_LOW_PRICE));
		message.addGroup(noMDEntryTypes);
		noMDEntryTypes.set(new MDEntryType(MDEntryType.TRADING_SESSION_VWAP_PRICE));
		message.addGroup(noMDEntryTypes);
		noMDEntryTypes.set(new MDEntryType(MDEntryType.TRADE_VOLUME));
		message.addGroup(noMDEntryTypes);

		MarketDataRequest.NoRelatedSym noRelatedSym = new MarketDataRequest.NoRelatedSym();
		Instrument instrument = new Instrument();
		instrument.set(new Symbol(symbol));
		noRelatedSym.set(instrument);
		message.addGroup(noRelatedSym);
		
		return message;
	}
}