export enum EntityType {
    Robot = 0,
    StaticObstacle = 1,
    DynamicObstacle = 2,
}

export interface Vec2 {
    x: number;
    y: number;
}

export interface EntityState {
    id: number;
    type: EntityType;
    x: number;
    y: number;
    heading: number;
    vertices: Vec2[];
    color: string;
}
