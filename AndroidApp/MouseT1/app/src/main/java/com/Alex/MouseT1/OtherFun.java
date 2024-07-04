package com.Alex.MouseT1;
import static android.content.Context.POWER_SERVICE;

import android.app.KeyguardManager;
import android.content.Context;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.Build;
import android.os.PowerManager;
import android.util.Log;

import androidx.annotation.RequiresApi;

import com.Alex.MouseT1.shell.Shell;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.nio.charset.StandardCharsets;
import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;
import android.os.StrictMode;

public class OtherFun {

    @RequiresApi(api = Build.VERSION_CODES.O)
    public static void unlockPhone(Context context, String postUrl) throws InterruptedException {

        KeyguardManager keyguardManager = (KeyguardManager) context.getSystemService(Context.KEYGUARD_SERVICE);
        if (keyguardManager.isKeyguardLocked()) {

            sendLargeJson("{\"type\":\"text\",\"text\":\"Unlocking phone\"}", postUrl);
            Log.d("testaa", "unlocking phone");
            try {
                Shell shell = new Shell();
                PowerManager powerManager = (PowerManager) context.getSystemService(POWER_SERVICE);
                if (!powerManager.isInteractive()) {
                    shell.executeRootCommand("input keyevent 26");
                }
                shell.executeRootCommand("input swipe 300 1000 300 500");
                shell.executeRootCommand("input text 3112");
                shell.executeRootCommand("input keyevent 66");
            } catch (Exception e) {
                Log.e("testaa", e.toString());
            }
            Thread.sleep(3000);
        }
    }



    public static void HttpPost(String jsonInputString, String url) {
        try {
            URL Url = new URL(url);
            HttpURLConnection connection = (HttpURLConnection) Url.openConnection();
            connection.setRequestMethod("POST");
            connection.setRequestProperty("Content-Type", "application/json; utf-8");
            connection.setRequestProperty("Accept", "application/json");
            connection.setDoOutput(true);

            try (OutputStream os = connection.getOutputStream()) {
                byte[] input = jsonInputString.getBytes(StandardCharsets.UTF_8);
                os.write(input, 0, input.length);
            }

            int responseCode = connection.getResponseCode();
            System.out.println("POST Response Code :: " + responseCode);

            if (responseCode == HttpURLConnection.HTTP_OK) {
                try (BufferedReader br = new BufferedReader(
                        new InputStreamReader(connection.getInputStream(), StandardCharsets.UTF_8))) {
                    StringBuilder response = new StringBuilder();
                    String responseLine = null;
                    while ((responseLine = br.readLine()) != null) {
                        response.append(responseLine.trim());
                    }
                    System.out.println(response);
                }
            } else {
                System.out.println("POST request not worked");
            }
            connection.disconnect();
        } catch (Exception e) {
           Log.d("testaa",e.toString());
        }
    }

    @RequiresApi(api = Build.VERSION_CODES.O)
    public static void sendLargeJson(String jsonString, String postUrl) throws InterruptedException {
        StrictMode.ThreadPolicy policy = new StrictMode.ThreadPolicy.Builder().permitAll().build();
        StrictMode.setThreadPolicy(policy);

        try {

            jsonString = jsonString.substring(0, jsonString.length() - 1) + ",\"time\":\"" + LocalDateTime.now().format(DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss.SSS")) + "\"}";

            int chunkSize = 1024;
            int totalLength = jsonString.length();
            int numberOfChunks = (int) Math.ceil((double) totalLength / chunkSize);

            for (int i = 0; i < numberOfChunks; i++) {
                int start = i * chunkSize;
                int end = Math.min(start + chunkSize, totalLength);
                String chunk = jsonString.substring(start, end);
                String chunkUrl = postUrl + "?chunk=" + i + "&total=" + numberOfChunks;
                OtherFun.HttpPost(chunk, chunkUrl);
            }
        }catch(Exception e){
            Log.e("testaa",e.toString());
        }
    }

    public static boolean isNetworkConnected(Context context) {
        StrictMode.ThreadPolicy policy = new StrictMode.ThreadPolicy.Builder().permitAll().build();
        StrictMode.setThreadPolicy(policy);

        ConnectivityManager cm = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
        if (cm != null) {
            NetworkInfo activeNetwork = cm.getActiveNetworkInfo();
            return activeNetwork != null && activeNetwork.isConnectedOrConnecting();
        }
        return false;
    }

    public static String readTextFromFile(File file) {
        StringBuilder stringBuilder = new StringBuilder();
        BufferedReader bufferedReader = null;

        try {
            bufferedReader = new BufferedReader(new FileReader(file));
            String line;

            while ((line = bufferedReader.readLine()) != null) {
                stringBuilder.append(line).append("\n");
            }
        } catch (IOException e) {
            Log.e("testaa",e.toString());
        } finally {
            try {
                if (bufferedReader != null) {
                    bufferedReader.close();
                }
            } catch (IOException e) {
                Log.e("testaa",e.toString());
            }
        }

        return stringBuilder.toString();
    }
    public static void writeJsonToFile(Context context, String fileName, String jsonContent) {
        File file = new File(context.getFilesDir(), fileName);
        try (FileOutputStream outputStream = new FileOutputStream(file)) {
            outputStream.write(jsonContent.getBytes());
            Log.d("testaa", "JSON content saved to " + fileName);
        } catch (IOException e) {
            Log.e("testaa", "Error writing JSON to file: " + e);
        }
    }

    public static String readJsonFromFile(Context context, String fileName) {
        File file = new File(context.getFilesDir(), fileName);
        StringBuilder jsonContent = new StringBuilder();

        try (FileInputStream inputStream = new FileInputStream(file);
             InputStreamReader inputStreamReader = new InputStreamReader(inputStream);
             BufferedReader bufferedReader = new BufferedReader(inputStreamReader)) {

            String line;
            while ((line = bufferedReader.readLine()) != null) {
                jsonContent.append(line);
            }

            Log.d("testaa", "JSON content read from " + fileName);
        } catch (IOException e) {
            Log.e("testaa", "Error reading JSON from file: " + e);
        }

        return jsonContent.toString();
    }
}