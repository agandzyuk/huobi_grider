package com.huobi.fix.util;

import java.io.UnsupportedEncodingException;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.Map;
import java.util.TreeMap;

public class MD5 
{
	// 全局数组
	private final static String[] strDigits = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "a", "b", "c", "d", "e", "f" };
	// 返回形式为数字跟字符串
	private static String byteToArrayString(byte bByte) 
	{
		int iRet = bByte;
		// System.out.println("iRet="+iRet);
		if (iRet < 0) 
		{
			iRet += 256;
		}
		int iD1 = iRet / 16;
		int iD2 = iRet % 16;
		return strDigits[iD1] + strDigits[iD2];
	}

	// 转换字节数组为16进制字串
	private static String byteToString(byte[] bByte) 
	{
		StringBuffer sBuffer = new StringBuffer();
		for (int i = 0; i < bByte.length; i++) 
		{
			sBuffer.append(byteToArrayString(bByte[i]));
		}
		return sBuffer.toString();
	}

	private static String GetMD5Code(String strObj) 
	{	
		try 
		{
			MessageDigest md = MessageDigest.getInstance("MD5");
			// md.digest() 该函数返回值为存放哈希值结果的byte数组
			return byteToString(md.digest(strObj.getBytes("utf-8")));
		} 
		catch (NoSuchAlgorithmException ex) {throw new RuntimeException();}
		catch (UnsupportedEncodingException e){throw new RuntimeException();}
	}
	
    private static Map<String,Object> signMap(TreeMap<String,Object> map)
    {
        map.put("created", System.currentTimeMillis()/1000);
        
        StringBuffer sb = new StringBuffer();
        
        for (Map.Entry<String, Object> me : map.entrySet())
        {
            sb.append(me.getKey()).append("=").append(me.getValue()).append("&");
        }
        
        String resultString = sb.substring(0, sb.length() - 1);
        String sign = GetMD5Code(resultString);
        map.put("sign", sign);
        
        return map;
    }
}
