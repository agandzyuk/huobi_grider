package com.huobi.fix.client.trade;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

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
import quickfix.UnsupportedMessageType;
import quickfix.field.ExecType;
import quickfix.field.Fee;
import quickfix.field.MsgType;
import quickfix.field.OrdStatus;
import quickfix.field.OrderID;
import quickfix.field.Password;
import quickfix.field.Price;
import quickfix.field.ProcessedAmount;
import quickfix.field.ProcessedPrice;
import quickfix.field.Quantity;
import quickfix.field.Side;
import quickfix.field.Total;
import quickfix.field.Username;
import quickfix.field.Vot;
import quickfix.fix44.AccountInfoResponse;
import quickfix.fix44.ExecutionReport;
import quickfix.fix44.HuobiOrderInfoResponse;
import quickfix.fix44.OrderCancelReject;
import quickfix.fix44.Reject;

public class TradeApplication extends MessageCracker implements Application {

    private Logger log = LoggerFactory.getLogger(getClass());

    private SessionID sessionID;
	private String accesskeyString;
    private String secretkeyString;

    public SessionID getSessionID() {
        return sessionID;
    }

	public String getAccesskeyString() {
		return accesskeyString;
	}

	public String getSecretkeyString() {
		return secretkeyString;
	}

	public TradeApplication(String accesskeyString, String secretkeyString) {
        this.accesskeyString = accesskeyString;
        this.secretkeyString = secretkeyString;
    }

    public boolean sendMessage(Message m) {
        if(this.sessionID == null) return false;
        return Session.lookupSession(sessionID).send(m);
    }

    public boolean sendMessage(Message m, SessionID sessionID) {
        if(sessionID == null) return false;
        return Session.lookupSession(sessionID).send(m);
    }

    @Override
    public void onCreate(SessionID sessionID) {
        this.sessionID = sessionID;
    }

    @Override
    public void onLogon(SessionID sessionID) {
        this.sessionID = sessionID;
        System.err.println("客户端登陆: " + sessionID.toString());
    }

    @Override
    public void onLogout(SessionID sessionID) {
        System.err.println("客户端登出: " + sessionID.toString());
    }

    @Override
    public void toAdmin(Message message, SessionID sessionID) {
    	
        try {
        	System.err.println("客户端发送的管理消息：" + message.toString());
            if (MsgType.LOGON.compareTo(message.getHeader().getString(MsgType.FIELD)) == 0) {
            	message.setString(Username.FIELD, accesskeyString);
            	message.setString(Password.FIELD, secretkeyString);
            }
        } catch (FieldNotFound e) {
            e.printStackTrace();
        }
    }

    @Override
    public void fromAdmin(Message message, SessionID sessionID) throws FieldNotFound, IncorrectDataFormat,
            IncorrectTagValue, RejectLogon {
        try {
        	System.out.println("客户端收到的管理消息：" + message.toString());
            if (message.getHeader().getString(MsgType.FIELD).equals("3")) crack(message, sessionID);
        } catch (UnsupportedMessageType e) {
            e.printStackTrace();
        }
    }

    @Override
    public void toApp(Message message, SessionID sessionID) throws DoNotSend {
    	System.err.println("客户端发送的应用消息：" + message.toString());
    }

    @Override
    public void fromApp(Message message, SessionID sessionID) throws FieldNotFound, IncorrectDataFormat,
            IncorrectTagValue, UnsupportedMessageType {
    	System.out.println("客户端收到的应用消息：" + message.toString());
        crack(message, sessionID);
    }

    public void onMessage(Reject reject, SessionID sessionID) {
		try {
			System.err.println("拒绝 sessionID: " + sessionID + " " + reject.getText().getValue());
		} catch (FieldNotFound e) {
			e.printStackTrace();
		}
    }
    
    public void onMessage(OrderCancelReject orderCancelReject, SessionID sessionID)
    {
		try {
			System.err.println("撤消订单拒绝 sessionID: " + sessionID + " " + orderCancelReject.getText().getValue());
		} catch (FieldNotFound e) {
			e.printStackTrace();
		}
    }

    // 账户信息
    public void onMessage(AccountInfoResponse accountInfoResponse, SessionID sessionID)
    {
       	try {
	   			System.err.println(new StringBuffer().append("总资产:" + accountInfoResponse.getTotal().getValue()).append("\n")
	   		   	.append("净资产:" + accountInfoResponse.getNetAsset().getValue()).append("\n")
	   			.append("可用金额:" + accountInfoResponse.getAvailableCny().getValue()).append("\n")
		    	.append("可用比特币:" + accountInfoResponse.getAvailableBtc().getValue()).append("\n")
		    	.append("可用莱特币:" + accountInfoResponse.getAvailableLtc().getValue()).append("\n")
		    	.append("冻结金额:" +  accountInfoResponse.getFrozenCny().getValue()).append("\n")
		    	.append("冻结比特币:" + accountInfoResponse.getFrozenBtc().getValue()).append("\n")
		    	.append("冻结莱特币:" + accountInfoResponse.getFrozenLtc().getValue()).append("\n")
		    	.append("己借金额:" + accountInfoResponse.getLoanCny().getValue()).append("\n")
		    	.append("己借比特币:" + accountInfoResponse.getLoanBtc().getValue()).append("\n")
		    	.append("己借莱特币:" + accountInfoResponse.getLoanLtc().getValue()).toString());
       	} catch (Exception e) {
       		e.printStackTrace();
       	}
	}
    
    // 火币订单信息
    public void onMessage(HuobiOrderInfoResponse huobiOrderInfoResponse, SessionID sessionID)
    {
        try {
            	System.err.println(new StringBuffer().append("委托订单:" + huobiOrderInfoResponse.getString(OrderID.FIELD) + "\n")
	            .append("交易类型:" + huobiOrderInfoResponse.getString(Side.FIELD) + "\n")
	            .append("委托价格:" + huobiOrderInfoResponse.getString(Price.FIELD) + "\n")
	            .append("委托数量:" + huobiOrderInfoResponse.getString(Quantity.FIELD) + "\n")
	            .append("成交均价:" + huobiOrderInfoResponse.getString(ProcessedPrice.FIELD) + "\n")
	            .append("已成交数量:" + huobiOrderInfoResponse.getString(ProcessedAmount.FIELD) + "\n")
	            .append("交易额:" + huobiOrderInfoResponse.getString(Vot.FIELD) + "\n")
	            .append("手续费:" + huobiOrderInfoResponse.getString(Fee.FIELD) + "\n")
	            .append("总交易额:" + huobiOrderInfoResponse.getString(Total.FIELD) + "\n")
	            .append("状态:" + huobiOrderInfoResponse.getString(OrdStatus.FIELD)).toString());
	            
        } catch (Exception e) {
        	e.printStackTrace();
        }
    }
    
    // 订单信息
    public void onMessage(ExecutionReport executionReport, SessionID sessionID)
    {
    	try {
        	switch (executionReport.getExecType().getValue()) {
 			
 			case ExecType.REJECTED:
 					System.err.println(new StringBuffer().append("执行失败, 错误代码: ").append(executionReport.getText().getValue()).toString());
 			    break;
 			    
 			case ExecType.NEW:
 					System.err.println(new StringBuffer().append("执行成功, 订单号: ").append(executionReport.getOrderID().getValue()).toString());
 			    break;
 			    
 			case ExecType.CANCELED:
 				System.err.println(new StringBuffer().append("取消成功, 订单号: ").append(executionReport.getOrderID().getValue()).toString());
 			    break;

 			case ExecType.ORDER_STATUS:
 				System.err.println(new StringBuffer().append("委托名称：" + executionReport.getSymbol().getValue()).append("\n")
 				    	.append("订单号:" + executionReport.getOrderID().getValue()).append("\n")
 				    	.append("委托数量:" + executionReport.getOrderQty().getValue()).append("\n")
 				    	.append("委托价格:" + executionReport.getPrice().getValue()).append("\n")
 				    	.append("未成交数量:" + executionReport.getLeavesQty().getValue()).append("\n")
 				    	.append("交易类型:" + (executionReport.getSide().getValue() == '1' ? "买" : "卖")).append("\n")
 				    	.append("订单状态:" + executionReport.getOrdStatus().getValue()).toString());
 			    break;
 			}
 		} catch (Exception e) {
 			e.printStackTrace();
 		}
    }
}
