package com.huobi.fix.client.market;

import java.io.InputStream;

import quickfix.ConfigError;
import quickfix.DefaultMessageFactory;
import quickfix.FileLogFactory;
import quickfix.FileStoreFactory;
import quickfix.Initiator;
import quickfix.LogFactory;
import quickfix.MessageStoreFactory;
import quickfix.SessionSettings;
import quickfix.SocketInitiator;

import com.huobi.fix.util.Constants;

public class MarketClient {

	public MarketClient() {
	}

	public static void main(String[] args) {
		try {
			MarketApplication application = new MarketApplication();
			InputStream inputStream = MarketClientApi.class.getClassLoader()
					.getResourceAsStream(Constants.CONF_FILEPATH_MARKETCLIENT);
			if (inputStream == null)
				throw new ConfigError();
			SessionSettings settings = new SessionSettings(inputStream);
			inputStream.close();
			MessageStoreFactory messageStoreFactory = new FileStoreFactory(
					settings);
			LogFactory logFactory = new FileLogFactory(settings);
			DefaultMessageFactory messageFactory = new DefaultMessageFactory();
			Initiator initiator = new SocketInitiator(application,
					messageStoreFactory, settings, logFactory, messageFactory);
			initiator.block();
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
}
