package com.huobi.fix.util;

public enum CoinType {
	BTCCoin(1), LTCCoin(2);
	private int i;

	private CoinType(int i) {
		this.i = i;
	}

	public static CoinType getTypeFromValue(int i) {
		for (CoinType item : values()) {
			if (item.i == i) {
				return item;
			}
		}

		return null;
	}
}
