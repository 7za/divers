
pub mod ece {
use std::sync::{Mutex, Arc};
use std::thread;
use std::fs::File;
use std::io::prelude::*;
use std::sync::mpsc;



/**
 * Basic type to store our datas
 */
//type  EceBuffer = Vec<u8>;
#[derive(Debug)]
struct EceBuffer {
    buffer: Vec<u8>,
    len   : usize,
}

/**
 * Mutable borrawable version of our buffer
 * We have to use it thru buffer to borrow it
 * in our buffer
 */
#[derive(Debug)]
struct EceBufferBank(Mutex<EceBuffer>);

impl EceBufferBank  {
    fn new(capacity: usize) -> Self {
        EceBufferBank(Mutex::new(
                EceBuffer { 
                            buffer: vec![0;capacity],
                            len: 0,
                          }))
    }

    /* apply a closure on our buffer
     * ensure that this is done thru lock
     */
    fn apply<F>(&self, mut action: F) where F:FnMut(&mut EceBuffer) {
        let  mut v = self.0.lock().unwrap();
        action(&mut v);
    }
}

/**
 * our bank of buffers (currently we only have two ones)
 */
#[derive(Debug)]
struct EceBuffers {
    banks: Vec<EceBufferBank>,
}


impl EceBuffers {
    /* create two buffers of capacity "capacity" */
    fn new(capacity: usize) -> Self {
        EceBuffers {
            banks : vec![EceBufferBank::new(capacity),
                         EceBufferBank::new(capacity)],
        }
    }

    /* Return a buffer on which we can apply a closure */
    fn get_bank(&self, bank_id : usize) -> &EceBufferBank {
        let slot = bank_id & 1;
        &self.banks[slot]
    }

    /* Directly apply a closure on selected bank */
    fn apply_bank<F>(&self, bank_id : usize, action: F)
    where F:FnMut(&mut EceBuffer) {
        let b = self.get_bank(bank_id);
        b.apply(action);
    }
}

#[derive(Debug)]
struct EceBuffersShared (Arc<EceBuffers>);

impl EceBuffersShared {
    fn new(capacity : usize) -> Self {
        EceBuffersShared(Arc::new(EceBuffers::new(capacity)))
    }
}

impl Clone for EceBuffersShared {
    fn clone(&self) -> EceBuffersShared {
        EceBuffersShared(Arc::clone(&self.0))
    }
}

#[derive(Debug)]
struct EceJob {
    offset: usize,
    len: usize,
    finished: bool,
    filename: Option<String>,
}

type EceJobMsgSnd = mpsc::Sender<EceJob>;
type EceJobMsgRcv = mpsc::Receiver<EceJob>;

#[derive(Debug)]
struct EceReader {
    buffer    : EceBuffersShared,
    thread    : Option<thread::JoinHandle<()>>,
    sender    : Option<EceJobMsgSnd>,
    ack       : EceJobMsgSnd,
    receiver  : Arc<Mutex<EceJobMsgRcv>>,
    id        : u32,
}

impl EceReader {
    fn new(id : u32, capacity : usize, listener: &EceJobMsgSnd ) -> Self {
        let (tx, rx) : (EceJobMsgSnd, EceJobMsgRcv) = mpsc::channel();
        EceReader{
                    buffer   : EceBuffersShared::new(capacity),
                    thread   : None,
                    sender   : Some(tx),
                    ack      : listener.clone(),
                    receiver : Arc::new(Mutex::new(rx)),
                    id: id,
        }
    }
}

impl Clone for EceReader {
    fn clone(& self) -> Self {
        EceReader {
            buffer   : EceBuffersShared(self.buffer.0.clone()),
            thread   : None,
            sender   : None,
            ack      : self.ack.clone(),
            receiver : Arc::clone(&self.receiver),
            id       : self.id,
        }
    }
}

#[derive(Debug)]
pub struct EceWorker {
    readers      : Vec<EceReader>,
    nrs_readers  : usize,
    capacity     : usize,
    listener     : EceJobMsgRcv,
}

impl Drop for EceWorker {
    fn drop(&mut self) { 
        let mut x : usize = 0;
        self.send_work(&mut x, 0, true); 
        for reader in &mut self.readers {
            match reader.thread.take()  {
                Some(r) => r.join().expect("joining thread"),
                None => println!("thread already joined ??"),
            }
        }
    }
}

impl EceWorker {

    fn wait_readers(&self)  {
        let mut job_done = 0;
        for _rx in &self.listener {
            job_done += 1;
            if job_done == self.nrs_readers {
                break;
            }
        }
    }

    fn open_files(&self, filenames : Vec <&'static str>) {
        for iter in filenames.iter().zip(self.readers.iter()) {
            let (filename, reader) = iter;
            let ece = EceJob { offset : 0,
                               len : 0,
                               finished: false,
                               filename: Some(filename.to_string()),
                            };
            match reader.sender {
                Some(ref u) => {
                    match u.send(ece) {
                        Ok(_) => {},
                        Err(_err) => println!("cannot send openfile cmd to worker {}", reader.id),
                    }
                },
                None => {},
            }
        }
    }

    fn send_work(&self, offset : &mut usize, capacity: usize, finished : bool) {
        for iter in self.readers.iter() {
            let ece = EceJob { offset : *offset,
                               len : self.capacity,
                               finished : finished,
                               filename: None,
                             };
            match iter.sender {
                Some(ref u) => {
                        match u.send(ece) {
                            Ok(_) => {},
                            Err(_err) => println!("cannot send new cmd to worker {}", iter.id),
                        }
                },
                None => {},
            }
        }
        *offset += capacity;
    }

    fn process_writer(&self, bank_read: &mut usize) -> usize {
        let mut len = 0;
        for reader in self.readers.iter() {
            reader.buffer.0.apply_bank(*bank_read,
                | buffer | {
                    len = buffer.len;
                    let s = String::from_utf8_lossy(&buffer.buffer);
                    println!("{}", s);
                }
            );
        }
        *bank_read = (*bank_read + 1) & 1;
        len
    }

    pub fn new(nr : usize, capacity: usize) -> Self {
        let (ack_tx, ack_rx) : (EceJobMsgSnd, EceJobMsgRcv) = mpsc::channel();
        let mut r = EceWorker {
            nrs_readers   : nr,
            readers       : (0..nr).map(|x| EceReader::new(x as u32, capacity, &ack_tx) ).collect(),
            capacity      : capacity,
            listener      : ack_rx,
        };
        for reader in r.readers.iter_mut() {
            let worker = reader.clone();
            let p = thread::spawn(move || {
                let mut first_bank = 0;
                let j = match worker.receiver.lock() {
                    Ok(lock) => lock,
                    Err(_err) => panic!("cannot lock"),
                };

                let mut f = None;
                loop {
                    let mut r = match j.recv() {
                        Ok(job) => job,
                        Err(_err) => EceJob {
                                        offset : 0,
                                        len : 0,
                                        finished : true,
                                        filename : None,
                                    },
                    };

                    if r.finished == true  {
                        break;
                    }
                    f = match r.filename {
                        Some(ref filename) => { Some(File::open(filename).expect("cannot open file"))},
                        None => f,
                    };

                    if r.len == 0  {
                        continue;
                    }

                    let mut bytes_read = 0;
                    if let Some(ref mut file) = f {
                        worker.buffer.0.apply_bank(first_bank, |buffer| {
                            match file.read(&mut buffer.buffer[..]) {
                                Ok(s) =>  { bytes_read = s; buffer.len = s },
                                Err(e) => { println!("cannot read content {}", e) },
                            }
                        });
                        r.len = bytes_read;
                        match worker.ack.send(r) {
                            Ok(_) => {},
                            Err(_) => {},
                        }
                        first_bank += 1;
                    }
                }
            });
            reader.thread = Some(p);
        }
        r    
    }

    pub fn work(&mut self, filenames : Vec<&'static  str>) {
        let mut offset = 0;
        let mut bank_read = 0;
         
        self.open_files(filenames);
        self.send_work(&mut offset, self.capacity, false);

        loop {
            self.wait_readers();

            self.send_work(&mut offset, self.capacity, false);

            let len = self.process_writer(&mut bank_read);
            if len < self.capacity {
                break;
            }

        }
    }
}

}



//fn main() {
//
//    let mut p = EceWorker::new(4, 128);
//    p.work(vec!["/tmp/1.txt", "/tmp/2.txt", "/tmp/3.txt", "/tmp/4.txt"]);
//
//    println!("second run");
//
//    p.work(vec!["/tmp/1.txt", "/tmp/2.txt", "/tmp/3.txt", "/tmp/4.txt"]);
//
//}

#[cfg(test)]
mod tests {
    #[test]
    fn do_4files() {
        let mut p = super::ece::EceWorker::new(4, 128);
        p.work(vec!["/tmp/1.txt", "/tmp/2.txt", "/tmp/3.txt", "/tmp/4.txt"]);
    }   
}

