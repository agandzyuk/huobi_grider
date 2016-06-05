package com.huobi.fix.client.trade;

import java.util.Date;
import java.util.UUID;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import quickfix.FieldNotFound;
import quickfix.field.ClOrdID;
import quickfix.field.EncryptMethod;
import quickfix.field.HeartBtInt;
import quickfix.field.MassStatusReqID;
import quickfix.field.MassStatusReqType;
import quickfix.field.MinQty;
import quickfix.field.OrdType;
import quickfix.field.OrigClOrdID;
import quickfix.field.Price;
import quickfix.field.Side;
import quickfix.field.Symbol;
import quickfix.field.Text;
import quickfix.field.TransactTime;
import quickfix.fix44.AccountInfoRequest;
import quickfix.fix44.Logon;
import quickfix.fix44.Logout;
import quickfix.fix44.NewOrderSingle;
import quickfix.fix44.OrderCancelRequest;
import quickfix.fix44.OrderMassStatusRequest;
import quickfix.fix44.OrderStatusRequest;

public class TradeClientApi {

	private Logger log = LoggerFactory.getLogger(getClass());
	private TradeApplication application;
	
	public TradeApplication getApplication() {
		return application;
	}

	public boolean isLogon() {
		return application.getSessionID() == null ? false : true;
	}

	public void logon() {
		Logon logon = new Logon();
		logon.set(new EncryptMethod());
		logon.set(new HeartBtInt());
		application.sendMessage(logon);
	}

	public void logout() {
		Logout logout = new Logout();
		logout.set(new Text("logout"));
		application.sendMessage(logout);
	}

	// 查询账户信息
	public boolean getAccountInfo() {
		AccountInfoRequest message = new AccountInfoRequest();
		message.set(new quickfix.field.AccReqID(UUID.randomUUID().toString()));
		message.set(new quickfix.field.Account(UUID.randomUUID().toString()));
		return application.sendMessage(message, application.getSessionID());
	}

	// 限价下单
	public boolean limitOrder(String symbol, double price, double amount,
			char side) throws FieldNotFound {
		if (price <= 0) {
			log.error("limitOrder price error");
			return false;
		}
		if (amount <= 0) {
			log.error("limitOrder amount error");
			return false;
		}
		if (symbol == null
				|| !(symbol.equalsIgnoreCase("BTC/CNY") || symbol
						.equalsIgnoreCase("LTC/CNY"))) {
			log.error("limitOrder symbol error");
			return false;
		}
		if (side != '1' && side != '2') {
			log.error("limitOrder side error");
			return false;
		}
		// 创建订单消息
		NewOrderSingle order = getOrderSingle();
		order.set(new Price(price));
		order.set(new MinQty(amount));
		order.set(new OrdType(OrdType.LIMIT));
		order.set(new Symbol(symbol));
		order.set(new Side(side));
		return application.sendMessage(order, application.getSessionID());
	}

	// 市价下单
	public boolean marketOrder(String symbol, double amount, char side)
			throws FieldNotFound {
		if (amount <= 0) {
			log.error("marketOrder amount error");
			return false;
		}
		if (symbol == null
				|| !(symbol.equalsIgnoreCase("BTC/CNY") || symbol
						.equalsIgnoreCase("LTC/CNY"))) {
			log.error("marketOrder symbol error");
			return false;
		}
		if (side != '1' && side != '2') {
			log.error("marketOrder side error");
			return false;
		}
		NewOrderSingle order = getOrderSingle();
		order.set(new Price(0));
		order.set(new OrdType(OrdType.MARKET));
		order.set(new MinQty(amount));
		order.set(new Symbol(symbol));
		order.set(new Side(side));
		return application.sendMessage(order, application.getSessionID());
	}

	// 修改订单
	public boolean modifyOrder(String orderID, String symbol, double price,
			double amount, char side) throws FieldNotFound {
		if (orderID == null) {
			log.error("modifyOrder orderID error");
			return false;
		}
		if (price <= 0) {
			log.error("modifyOrder price error");
			return false;
		}
		if (amount <= 0) {
			log.error("modifyOrder amount error");
			return false;
		}
		if (symbol == null
				|| !(symbol.equalsIgnoreCase("btc") || symbol
						.equalsIgnoreCase("ltc"))) {
			log.error("modifyOrder symbol error");
			return false;
		}
		if (side != '1' && side != '2') {
			log.error("modifyOrder side error");
			return false;
		}
		// 创建订单消息
		NewOrderSingle order = getOrderSingle();
		order.set(new ClOrdID(orderID));
		order.set(new Price(price));
		order.set(new MinQty(amount));
		order.set(new OrdType(OrdType.LIMIT_OR_BETTER));
		order.set(new Symbol(symbol));
		order.set(new Side(side));
		return application.sendMessage(order, application.getSessionID());
	}

	// 取消委托
	public boolean cancelOrder(String orderID, String symbol, char side)
			throws FieldNotFound {
		if (orderID == null || orderID.equals("")) {
			log.error("cancelOrder orderID error");
			return false;
		}
		if (symbol == null
				|| !(symbol.equalsIgnoreCase("btc") || symbol
						.equalsIgnoreCase("ltc"))) {
			log.error("cancelOrder symbol error");
			return false;
		}
		if (side != '1' && side != '2') {
			log.error("cancelOrder side error");
			return false;
		}
		OrderCancelRequest orderCancelRequest = new OrderCancelRequest(
				new OrigClOrdID(orderID), new ClOrdID(UUID.randomUUID()
						.toString()), new Side(), new TransactTime(new Date()));
		orderCancelRequest.set(new Symbol(symbol));
		orderCancelRequest.set(new Side(side));
		return application.sendMessage(orderCancelRequest,
				application.getSessionID());
	}

	// 获取所有正在进行的委托
	public boolean getOrders(String symbol) {
		if (symbol == null
				|| !(symbol.equalsIgnoreCase("BTC/CNY") || symbol
						.equalsIgnoreCase("LTC/CNY"))) {
			log.error("queryOrders symbol error");
			return false;
		}
		OrderMassStatusRequest orderMassStatusRequest = new OrderMassStatusRequest(
				new MassStatusReqID(UUID.randomUUID().toString()),
				new MassStatusReqType(MassStatusReqType.STATUS_FOR_ALL_ORDERS));
		orderMassStatusRequest.set(new Symbol(symbol));
		return application.sendMessage(orderMassStatusRequest,
				application.getSessionID());
	}

	// 查询订单
	public boolean getOrder(String symbol, String orderID, char side) {
		if (orderID == null || orderID.equals("")) {
			log.error("queryOrder orderID error");
			return false;
		}
		if (symbol == null
				|| !(symbol.equalsIgnoreCase("BTC/CNY") || symbol
						.equalsIgnoreCase("LTC/CNY"))) {
			log.error("queryOrder symbol error");
			return false;
		}
		if (side != '1' && side != '2') {
			log.error("queryOrder side error");
			return false;
		}
		OrderStatusRequest order = new OrderStatusRequest();
		order.set(new ClOrdID(orderID));
		order.set(new Symbol(symbol));
		order.set(new Side(side));
		return application.sendMessage(order, application.getSessionID());
	}

	private NewOrderSingle getOrderSingle() {
		NewOrderSingle order = new NewOrderSingle();
		order.set(new ClOrdID(UUID.randomUUID().toString()));
		order.set(new TransactTime(new Date()));
		order.set(new OrdType(OrdType.LIMIT));
		return order;
	}
}