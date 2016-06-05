package com.huobi.fix.client.trade;

import java.io.IOException;
import java.io.InputStream;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import quickfix.ConfigError;
import quickfix.DefaultMessageFactory;
import quickfix.FileLogFactory;
import quickfix.FileStoreFactory;
import quickfix.Initiator;
import quickfix.LogFactory;
import quickfix.MessageStoreFactory;
import quickfix.SessionSettings;
import quickfix.SocketInitiator;

import com.huobi.fix.client.market.MarketApplication;
import com.huobi.fix.client.market.MarketClientApi;
import com.huobi.fix.util.Constants;

public class TradeClient {
	private static Logger logger = LoggerFactory.getLogger(TradeClient.class);

	public static void main(String[] args) throws IOException {
		String accessKey = args.length < 1 ? "5ff03d48-468f1e3f-9e9d9bd1-681b3"
				: args[0];
		String secretKey = args.length < 2 ? "3dc36d7e-1fbbb90d-f9d2bd2e-39224"
				: args[1];
		try {
			TradeApplication application = new TradeApplication(accessKey, secretKey);
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
