package com.huobi.fix.util;

public enum TradeType {
	Buy(1), Sell(2);
	private int i;

	private TradeType(int i) {
		this.i = i;
	}

	public static TradeType getTypeFromValue(int i) {
		for (TradeType item : values()) {
			if (item.i == i) {
				return item;
			}
		}
		return null;
	}
}
