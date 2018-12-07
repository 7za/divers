use std::rc::{Weak,Rc};
use std::cell::RefCell;


type WeakRef<T> = Weak<RefCell<ListNode<T>>>;
type StrongRef<T> = Rc<RefCell<ListNode<T>>>;

#[derive(Debug)]
struct ListNode<T> {
    parent:   Option<WeakRef<T>>,
    children: Vec<StrongRef<T>>,
    value: T,
}

#[derive(Debug)]
struct ListNodeRef<T>(StrongRef<T>);


impl<T> Clone for ListNodeRef<T> {
    fn clone(&self) -> ListNodeRef<T> {
        ListNodeRef(Rc::clone(&self.0))
    }
}

impl<T> Drop for ListNodeRef<T> {
    fn drop(&mut self) {
        println!("refcount={}, weakcount={}", Rc::strong_count(&self.0), Rc::weak_count(&self.0));
    }
}

impl<T> ListNodeRef<T> {
    fn new(value: T) -> Self {
        ListNodeRef(Rc::new(
                    RefCell::new(
                        ListNode{ parent: None,
                        children:vec![],
                        value:value
                        }
                        )    
                    ))
    }

    fn get_parent(&self) -> Option<ListNodeRef<T>> {
        match self.0.borrow().parent.as_ref() {
            None => None,

            Some(ref l) => {
                     match l.upgrade() {
                         Some(f) => Some(ListNodeRef(f)),
                         None => None,
                     }

            }
        }
    }

    fn add_children(&self, child: &ListNodeRef<T>) {

        /* check parent existance */
        match child.get_parent() {
            Some(_) => println!("parent already set"),

            None => {
                child.0.borrow_mut().parent = Some(Rc::downgrade(&self.0));
                self.0.borrow_mut().children.push(Rc::clone(&child.0))
            }
        }
    }

    fn equals(&self, other: &ListNodeRef<T>) -> bool { Rc::ptr_eq(&self.0, &other.0) }

}

fn main() {
    let p : ListNodeRef<&str> = ListNodeRef::new("child");

    println!("lparent = {:?}", p.get_parent());
    let nl : ListNodeRef<&str> = ListNodeRef::new("nparent");
    nl.add_children(&p);
    println!("lparent = {:?}", p.get_parent());

    let gparent = p.get_parent();
    match gparent {
        Some(ref node) => 
            println!("gparent and nl : {} {}", node.equals(&nl), nl.equals(&node)),
        None => println!("cannot get parent data"),
    }
}
