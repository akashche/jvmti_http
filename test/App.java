/*
 * Copyright 2015, akashche at redhat.com
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
import java.util.Random;

class App {
    public static void main(String[] args) throws Exception {
        System.out.println("Java app started and running...");
        final Random ra = new Random(42);
        for(;;) {
            for (int i = 0; i < 10; i++) {
                new Thread(new Runnable() {
                    public void run() {
                        try {
                            Thread.sleep((int) (ra.nextFloat() * 1000));
                        } catch (InterruptedException ex) {
                            throw new RuntimeException(ex);
                        }
                    }
                }).start();
            }
            Thread.sleep(1000);
        }
    }
}
