package com.ytta.v0;

import com.ytta.v0.Types.Side;
import com.ytta.v0.Types.Tick;

import java.io.BufferedReader;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.List;

public final class TickSource {
    private TickSource() {}

    public static List<Tick> load(Path path) throws IOException {
        List<Tick> ticks = new ArrayList<>();
        try (BufferedReader reader = Files.newBufferedReader(path)) {
            String line;
            int lineNo = 0;
            while ((line = reader.readLine()) != null) {
                lineNo++;
                Tick t = parseTickLine(line);
                if (t != null) {
                    ticks.add(t);
                } else if (!line.isBlank() && !line.trim().startsWith("#")) {
                    String type = jsonString(line, "type");
                    if (type != null && !"tick".equals(type)) {
                        continue;
                    }
                    if (type != null) {
                        throw new IOException("malformed tick at line " + lineNo);
                    }
                }
            }
        }
        return ticks;
    }

    static Tick parseTickLine(String line) {
        if (line == null || line.isBlank() || line.trim().startsWith("#")) {
            return null;
        }
        String type = jsonString(line, "type");
        if (!"tick".equals(type)) {
            return null;
        }
        long tsNs = jsonLong(line, "ts_ns");
        long price = jsonLong(line, "price");
        long qty = jsonLong(line, "qty");
        long seq = jsonLong(line, "seq");
        String instrument = jsonString(line, "instrument");
        String sideS = jsonString(line, "side");
        if (instrument == null || sideS == null) {
            throw new IllegalArgumentException("malformed tick");
        }
        Side side = switch (sideS) {
            case "BUY" -> Side.BUY;
            case "SELL" -> Side.SELL;
            default -> throw new IllegalArgumentException("invalid side " + sideS);
        };
        return new Tick(tsNs, instrument, side, price, qty, seq);
    }

    private static String jsonString(String line, String key) {
        String needle = "\"" + key + "\":\"";
        int start = line.indexOf(needle);
        if (start < 0) {
            return null;
        }
        start += needle.length();
        int end = line.indexOf('"', start);
        if (end < 0) {
            return null;
        }
        return line.substring(start, end);
    }

    private static long jsonLong(String line, String key) {
        String needle = "\"" + key + "\":";
        int start = line.indexOf(needle);
        if (start < 0) {
            throw new IllegalArgumentException("missing " + key);
        }
        start += needle.length();
        while (start < line.length() && Character.isWhitespace(line.charAt(start))) {
            start++;
        }
        int end = start;
        if (end < line.length() && (line.charAt(end) == '-' || line.charAt(end) == '+')) {
            end++;
        }
        while (end < line.length() && Character.isDigit(line.charAt(end))) {
            end++;
        }
        return Long.parseLong(line.substring(start, end));
    }
}
